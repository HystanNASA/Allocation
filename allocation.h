#ifndef _ALLOCATION_H
#define _ALLOCATION_H

#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define META_SIZE sizeof(struct block_meta)

struct block_meta
{
    size_t size;
    struct block_meta* next;
    int ffree;
    /* Is used for debugging. Can be removed */
    int magic;
};

struct block_meta* find_free_block(struct block_meta**, size_t);
struct block_meta* ewquest_space(struct block_meta*, size_t);
struct block_meta* get_block_ptr(void*);

void ffree(void*);
void* mmalloc(size_t);
void* rrealloc(void*, size_t);
void* ccalloc(size_t, size_t);

#endif