#include "allocation.h"

struct block_meta* global_base = NULL;
short first_allocation = 0;

void print_state()
{
    struct block_meta* current_block = global_base;

    while(current_block)
    {
        printf("block\n");

        if(current_block->next)
            current_block = current_block->next;
        else
            break;
    }
}

struct block_meta* find_free_block(size_t size)
{
#if __x86_64__ || __ppc64__
    #define macro_int int64_t
#else
    #define macro_int int32_t
#endif


    if(!global_base)
        return NULL;

    struct block_meta* current_block = global_base;

    /* Looking for free block */
    while(current_block->next && !(current_block->free && current_block->size >= size)) current_block = current_block->next;

    /* NOTE: Used for testing */
    assert(current_block);

    /* Try to split the block */
    /* Do we have enough space after splitting the block? If so, is it divided by 2? */
    if( ( (macro_int)current_block->size - (macro_int)size - (macro_int)SIZE_OF_BLOCK_META > 1 )
            && ( ( (macro_int)current_block->size - (macro_int)size - (macro_int)SIZE_OF_BLOCK_META) % 2) == 0 )
    {
        struct block_meta* new_block = current_block->size + SIZE_OF_BLOCK_META;

        new_block->size     = size;
        new_block->next     = current_block->next;
        new_block->prev     = current_block;
        new_block->free     = 1;

        current_block->size = current_block->size - size - SIZE_OF_BLOCK_META;
        current_block->next = new_block;

        // NOTE: segfault because of new_block locating
        //memset(new_block + SIZE_OF_BLOCK_META, 0, new_block->size);

        printf("splitting\n");
    }

    current_block->free = 0;

    //memset(current_block + SIZE_OF_BLOCK_META, 0, current_block->size);

#undef macro_int
    
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
        new_block       = sbrk(SIZE_OF_BLOCK_META + size);
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
        new_block->size     = size;
        new_block->next     = NULL;
        new_block->prev     = current_block;
        new_block->free     = 0;

        current_block->next = new_block;
        
        return new_block;
    }
}

void merge()
{
    if(!global_base)
        return;

    struct block_meta* current_block = global_base;
    struct block_meta* last_block = NULL;

    if(!current_block->next)
        printf("We don't have more that one block\n");
    else
        printf("We have more that one block\n");

    while(current_block->next)
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

void* get_block_meta(void* ptr)
{
    return (ptr - SIZE_OF_BLOCK_META);
}

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

    if(!first_allocation)
    {
        first_allocation = 1;
        /* cleanup() will be called at normal process termination */
        atexit(cleanup);
    }

    struct block_meta* new_block;

    /* Do we have blocks? */
    if(!global_base)
    {
        new_block = global_base = request_spcae(size);
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
        
    struct block_meta* ptrs_block = get_block_meta(ptr);

    if(ptrs_block->size >= size)
        return ptr;

    ptrs_block->free = 1;

    return mmalloc(size);
}

void ffree(void* ptr)
{
    if(!ptr)
        return;

    struct block_meta* ptrs_block = get_block_meta(ptr);

    ptrs_block->free = 1;

    merge();
    print_state();
}