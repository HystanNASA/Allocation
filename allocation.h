#ifndef ALLOCATION_H
#define ALLOCATION_H

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>  // For debugging. Remove it later
#include <assert.h> // For debugging. Remove it later

#define SIZE_OF_BLOCK_META sizeof(struct block_meta)

struct block_meta
{
    size_t size;
    struct block_meta* next;
    struct block_meta* prev;
    short free;
};

void* mmalloc(size_t);
void* ccalloc(size_t, size_t);
void* rrealloc(void*, size_t);
void ffree(void*);
void cleanup();

#endif