#ifndef ALLOCATION_H
#define ALLOCATION_H

#include <stddef.h>
#include <unistd.h>
#include <string.h>

#define SIZE_OF_BLOCK_META sizeof(struct block_meta)

/* 18 bytes on 64-bit */
struct block_meta
{
    size_t size;
    struct block_meta* next;
    short free;
};

struct block_meta* find_free_block(struct block_meta**, size_t);
struct block_meta* request_space(struct block_meta*, size_t);
struct block_meta* get_block_meta(void*);

void* mmalloc(size_t);
void* ccalloc(size_t, size_t);
void* rrealloc(void*, size_t);
void ffree(void*);


#endif