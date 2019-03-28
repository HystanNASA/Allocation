#include "allocation.h"

void* global_base = NULL;

void* mmalloc(size_t size)
{
    struct block_meta* new_block;

    if(size == 0)
        return NULL;

    if(!global_base) /* Haven't allocated anything */
    {
        new_block = global_base = request_space(NULL, size);
        if(!global_base)
            return NULL;
    }
    else /* Try to find a free block, or allocate a new one */
    {
        new_block = find_free_block(global_base, size);
        
        if(!new_block)
        {
            new_block = request_space(global_base, size);
                if(!new_block)
                    return NULL;
            new_block->size = size;
            new_block->next = NULL;
            new_block->free = 0;
        }
    }

    /* Return the region after block_meta */
    return (new_block + 1);
}

void ffree(void* ptr)
{
    if(!ptr)
        return;

    struct block_meta* ptrs_block = get_block_meta(ptr);
    
    if(ptrs_block->free == 0)
        return;
    else
        ptrs_block->free = 0;
}

struct block_meta* find_free_block(struct block_meta** last_block, size_t size)
{
    struct block_meta* current_block = global_base;

    /* Find free block. It will return either NULL or a pointer */
    while(current_block && !(current_block->free && current_block->size >= size))
    {
        *last_block    = current_block;
        current_block = current_block->next;
    }

    return current_block;
}

struct block_meta* request_space(struct block_meta* last_block, size_t size)
{
    struct block_meta*  new_block   = sbrk(0);
    void*               request     = sbrk(size + SIZE_OF_BLOCK_META);

    if( ((void*)new_block != request) || (request == (void*)-1) )
        return NULL;

    if(last_block)
        last_block->next = new_block;

    new_block->size = size;
    new_block->next = NULL;
    new_block->free = 0;
    return new_block;
}

struct block_meta* get_block_meta(void* ptr)
{
    return (struct block_meta*)ptr - 1;
}