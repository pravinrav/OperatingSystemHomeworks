/*
 * mm_alloc.c
*/

#include "mm_alloc.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

struct block {
    struct block * previous;
    struct block * next;

    size_t size;
    int free;
};

struct block head;


struct block * findFreeBlock(size_t size) {
    
    // Search list for block that is large enough to fit the size
    struct block * b = &head;
    while (b != NULL) {

        if (b->free) {
            if (b->size > sizeof(struct block) + size) {
                return b;
            }
        }
        b = b->next;
    }

    return NULL;
}

static void * findMallocBlock(size_t size) {

  struct block * freeBlock = findFreeBlock(size);

  if (freeBlock == NULL) return NULL;

  // Check if we have to partition the block
  if (freeBlock->size > 2 * sizeof(struct block) + size) {
    struct block * new = (struct block *) freeBlock + size + sizeof(struct block);
    new->previous = freeBlock;
    new->next = freeBlock->next;

    new->size = freeBlock->size - sizeof(struct block) - size;
    new->free = 1;

    freeBlock->size = size;
    freeBlock->next = new;
    freeBlock->free = 0;
  } else { // We don't have to partition the block
    freeBlock->size = size;
    freeBlock->free = 0;
  }

  return (void *) freeBlock + sizeof(struct block);

}

void *mm_malloc(size_t size) {
    //TODO: Implement malloc
    if (size == 0) {
        return NULL;
    }

    void * mallocPtr = findMallocBlock(size);
    if (mallocPtr != NULL) {
        return mallocPtr; 
    }

    struct block * currentEnd = (struct block *) sbrk(0);
    sbrk(sizeof(struct block) + size);

    currentEnd->size = size; 
    currentEnd->free = 0; 

    // insert the struct block into the linked list
    struct block * tailEnd = &head;
    tailEnd->next = currentEnd;
    currentEnd->previous = tailEnd;
    currentEnd->next = NULL;
    tailEnd = currentEnd;

    mallocPtr = (void *) (sizeof(struct block) + currentEnd);
    bzero(mallocPtr, size);

    return mallocPtr;
}

void *mm_realloc(void *ptr, size_t size) {
    return NULL;
}

static void checkValidMallocPointer(void * ptr) {
    struct block * b = head.next;
    while (b != NULL) {
        void * start = (void *) b + sizeof(struct block);
        if (start == ptr) {
            return;
        }
        b = b->next;
    }
}

void mm_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    checkValidMallocPointer(ptr);

    struct block * currBlock = (struct block *) (ptr - sizeof(struct block));
    currBlock->free = 1; 

    struct block * nextBlock = currBlock->next;

    while ((nextBlock != NULL) && (nextBlock->free == 1)) {

        if (nextBlock->next == 0) {
            currBlock->next = NULL;
            currBlock->size = sbrk(0) - sizeof(struct block) - (void *) currBlock;
        }

        else {
            currBlock->size = nextBlock->next - sizeof(struct block) - currBlock;
            currBlock->next = nextBlock->next;

            nextBlock->previous = currBlock;
        }

        nextBlock = currBlock->next;
    }



}