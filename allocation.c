#include "allocation.h"

/* Head of the linked list */
void* global_base = NULL;

struct block_meta* find_free_block(struct block_meta** last, size_t size)
{
    struct block_meta* current = global_base;
    while(current && !(current->ffree && current->size >= size))
    {
        *last = current;
        current = current->next;
    }
    return current;
}

struct block_meta* request_space(struct block_meta* last, size_t size)
{
    struct block_meta* block = sbrk(0);
    void* request = sbrk(size + META_SIZE);
    assert((void*)block == request);
    
    if(request == (void*)-1)
    {
        return NULL;
    }

    if(last)
    {
        last->next = block;
    }

    block->size = size;
    block->next = NULL;
    block->ffree = 0;
    block->magic = 0x12345678;
    
    return block;
}

struct block_meta* get_block_ptr(void* ptr)
{
    return (struct block_meta*)ptr - 1;
}

void ffree(void* ptr)
{
    /* Is it already fffree? */
    if(!ptr)
        return;

    struct block_meta* block_ptr = get_block_ptr(ptr);
    assert(block_ptr->ffree == 0);
    assert(block_ptr->magic == 0x77777777 || block_ptr->magic == 0x12345678);
    block_ptr->ffree = 1;
    block_ptr->magic = 0x55555555;
}

void* mmalloc(size_t size)
{
    struct block_meta* block;

    if(size <= 0)
    {
        return NULL;
    }

    /* First call */
    if(!global_base)
    {
        block = request_space(NULL, size);
        if(!block)
        {
            return NULL;
        }
    }
    else
    {
        struct block_meta* last = global_base;
        block = find_free_block(&last, size);

        /* Failed to find ffree block */
        if(!block)
        {
            block = request_space(last, size);
            if(!block)
            {
                return NULL;
            }
        }
        else
        {
            block->ffree = 0;
            block->magic = 0x77777777;
        }
    }

    return(block + 1);
}

void* rrealloc(void* ptr, size_t size)
{
    /* If ptr is NULL, it acts like malloc */
    if(!ptr)
        return mmalloc(size);

    struct block_meta* block_ptr = get_block_ptr(ptr);
    /* We have enough space. Could we free some once we implement split */
    if(block_ptr->size >= size)
        return ptr;

    /* Malloc new space and free old space.
       The copy old data to new space.
    */
    void *new_ptr = mmalloc(size);
    if(!new_ptr)
        return NULL;
    
    memset(new_ptr, ptr, block_ptr->size);
    ffree(ptr);
    return new_ptr;
}

void* ccalloc(size_t nelem, size_t elsize)
{
    size_t size = nelem * elsize;
    void* ptr = mmalloc(size);
    memset(ptr, 0, size);
    return ptr;
}