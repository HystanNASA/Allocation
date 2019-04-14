#include "allocation.h"

#define GET_BLOCK_META(ptr) (struct block_meta*)((struct block_meta*)ptr - SIZE_OF_BLOCK_META)
#define MAX_TOTAL_SIZE 512 /* If the total size of blocks more than this, they are deleted */

struct block_meta* global_base = NULL;
short first_allocation = 0;

void print_state()
{
    struct block_meta* current_block = global_base;

    printf("<heap>\n");
    for(int i = 0; current_block; i++)
    {
        printf("%d. block free %d\n", i, current_block->free);

        if(current_block->next)
            current_block = current_block->next;
        else
            break;
    }
    printf("</heap>\n");
}

/* 
This function splits a large block.
If the block is splitable, the block variable has the requested size.
It doesn't reutrn anything because that info is useless.
*/
void split_large_block(struct block_meta *const block, const size_t size)
{
    if(!block)
        return;

    /* Will we have enough space after splitting the block? If so, is it divided by 2? */
    if( ( block->size - size - SIZE_OF_BLOCK_META > 1 )
            && ( ( block->size - size - SIZE_OF_BLOCK_META) % 2) == 0 )
    {
        struct block_meta* new_block = block + size + SIZE_OF_BLOCK_META;

        new_block->size = block->size - size - SIZE_OF_BLOCK_META;
        new_block->next = block->next;
        new_block->prev = block;
        new_block->free = 1;

        block->size = size;
        block->next = new_block;

        memset(new_block + SIZE_OF_BLOCK_META, 0, new_block->size);
    }
}

struct block_meta* find_free_block(size_t size)
{
    if(!global_base)
        return NULL;

    struct block_meta* current_block = global_base;

    /* Looking for a free block */
    while(current_block->next && !(current_block->free && current_block->size >= size)) current_block = current_block->next;

    /* NOTE: Used for testing */
    assert(current_block);

    if(current_block->free == 0)
        return NULL;

    /* Try to split the block */
    split_large_block(current_block, size);
    
    current_block->free = 0;

    memset(current_block + SIZE_OF_BLOCK_META, 0, current_block->size);

    return current_block;
}

struct block_meta* request_spcae(size_t size)
{
    if(size == 0)
        return NULL;

    struct block_meta* current_block = global_base;
    struct block_meta* new_block = NULL;

    if(!global_base)
    {
        new_block = sbrk(SIZE_OF_BLOCK_META + size);

        /*NOTE: Used for testing*/
        assert(new_block != (void*)-1);

        new_block->size = size;
        new_block->next = NULL;
        new_block->prev = NULL;
        new_block->free = 0;

        return new_block;
    }

    /* Looking for the last block */
    while(current_block->next) current_block = current_block->next;

    /* Is the last block free? */
    if(current_block->free)
    {
        /* Does it match the requested size? */
        if(current_block->size >= size)
        {
            current_block->free = 0;
            return current_block;
        }
        else
        {
            /* Expand the size */
            sbrk(size - current_block->size);
            
            current_block->size = size;
            current_block->free = 0;

            return current_block;
        }
    }
    else
    {
        new_block           = sbrk(SIZE_OF_BLOCK_META + size);

        /*NOTE: Used for testing*/
        assert(new_block != (void*)-1);

        new_block->size     = size;
        new_block->next     = NULL;
        new_block->prev     = current_block;
        new_block->free     = 0;

        current_block->next = new_block;
        
        return new_block;
    }
}

/* This function merges blocks */
void merge()
{
    if(!global_base)
        return;

    struct block_meta* current_block = global_base;
    struct block_meta* last_block = NULL;

    while(current_block)
    {
        if(current_block->free && last_block && last_block->free)
        {
            last_block->size += current_block->size + SIZE_OF_BLOCK_META;
            last_block->next = current_block->next;

            memset(last_block + SIZE_OF_BLOCK_META, 0, last_block->size);
            
            if(last_block->next)
                current_block = last_block->next;
            else
                return;
        }
        else
        {
            last_block = current_block;
            current_block = current_block->next;
		}
    }
}

/* This function deletes blocks if they take a lot of space. */
void check_and_delete_blocks()
{
	if (!global_base || global_base->next == NULL)
		return;

	struct block_meta* last_block = NULL;
	size_t total_size = 0;

	/* Looking for the last block */
	for (last_block = global_base->next; last_block->next; last_block = last_block->next);
		total_size += (size_t)last_block - (size_t)global_base;

	struct block_meta* prev_block = last_block->prev;
	
	/* Delete blocks if their size more than MAX_TOTAL_SIZE */
	while ( (total_size >= MAX_TOTAL_SIZE) && (last_block) && (last_block->free) )
	{
		total_size -= last_block->size + SIZE_OF_BLOCK_META;
		sbrk( -(intptr_t)( last_block->size + SIZE_OF_BLOCK_META) );

		last_block = prev_block;
	}
}

/* This function sets brk() to the first position, i.e., cleaning memory up. */
void cleanup()
{
    if(global_base)
        brk(global_base);

    global_base = NULL;
}

void* mmalloc(size_t size)
{
    if(size == 0)
        return NULL;

    if(first_allocation == 0)
    {
        first_allocation = 1;
        /* cleanup() will be called at normal process termination */
        atexit(cleanup);
    }

    struct block_meta* new_block;

    /* Do we have blocks? */
    if(!global_base)
    {
        global_base = new_block = request_spcae(size);
        if(!new_block)
        {
            global_base = NULL;
            return NULL;
        }
    }
    else
    {
        new_block = find_free_block(size);

        /* Did we find a free block? */
        if(!new_block)
        {
            /* Allocating a new one */
            new_block = request_spcae(size);
            if(!new_block)
                return NULL;
		}
    }

    return (new_block + SIZE_OF_BLOCK_META);
}

void* ccalloc(size_t num, size_t size)
{
    if(num == 0 || size == 0)
        return NULL;

    return mmalloc(num * size);
}

void* rrealloc(void* ptr, size_t size)
{
    if(!ptr || size == 0)
        return NULL;
        
    struct block_meta* ptrs_block = GET_BLOCK_META(ptr);

    if(ptrs_block->size >= size)
        return ptr;

    ptrs_block->free = 1;

    return mmalloc(size);
}

void ffree(void* ptr)
{
    if(!ptr)
        return;

	struct block_meta* ptrs_block = GET_BLOCK_META(ptr);
    ptrs_block->free = 1;

	check_and_delete_blocks();
    merge();
}