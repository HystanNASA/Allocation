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

void* ccalloc(size_t num, size_t size)
{
    if(num == 0 || size == 0)
        return NULL;

    return mmalloc(num * size);
}

void* rrealloc(void* ptr, size_t size) /* Copy old data */
{
    if(size == 0)
        return NULL;
    else if(!ptr)
        return mmalloc(size);

    struct block_meta* ptrs_block = get_block_meta(ptr);

    /* Do we have enough space? */
    if(ptrs_block->size >= size)
        return ptr;

    void* new_ptr = mmalloc(size);
    if(!new_ptr)
        return NULL;

    memcpy(new_ptr, ptr, size);
    return new_ptr;
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