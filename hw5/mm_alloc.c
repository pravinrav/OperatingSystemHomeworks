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

struct block * head = NULL;


struct block * findFreeBlock(size_t size) {
    
    // Search list for block that is large enough to fit the size
    struct block * b = head;
    while (b != NULL) {

        if (b->free) {
            if (b->size > size) {
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

    if (freeBlock->next) {
        freeBlock->next->previous = new; 
    }

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

    void * value = sbrk(sizeof(struct block) + size);
    if (value == (void *) -1) {
        return NULL; 
    }

    currentEnd->size = size; 
    currentEnd->free = 0; 

    // insert the struct block into the linked list
    if (head == NULL) {
        head = currentEnd;
    } 
    else {

        // iterate through the memory linked list to find the last element
        struct block * b = head;
        while (b->next != NULL) {
            b = b->next;
        }

        b->next = currentEnd;
        currentEnd->previous = b;
        currentEnd->next = NULL;

    }

    mallocPtr = (void *) (sizeof(struct block) + currentEnd);
    bzero(mallocPtr, size);

    return mallocPtr;
}


static int checkValidMallocPointer(void * ptr) {
    struct block * b = head;
    while (b != NULL) {
        void * start = (void *) b + sizeof(struct block);
        if (start == ptr) {
            return 1;
        }
        b = b->next;
    }
    return 0;
}

void *mm_realloc(void *ptr, size_t size) {
    if (ptr == NULL && size != 0) {
        return mm_malloc(size);
    }
    if (size == 0 && ptr != NULL) {
        mm_free(ptr);
        return NULL;
    }
    if (ptr == NULL && size == 0) {
        return NULL;
    }

    checkValidMallocPointer(ptr);

    struct block * currBlock = (struct block *) (ptr - sizeof(struct block));
    struct block * nextBlock = currBlock->next;

    // Is the currBlock large enough to handle the new size?
    if (size < ( (void *) nextBlock - (void *) currBlock - sizeof(struct block) ) ) {

        void * start = (void *) (currBlock + size);
        bzero(start, currBlock->size - size);

        currBlock->size = size;

        return ptr;
    }

    // Otherwise allocate a new block with the new size 
    struct block * newBlock = mm_malloc(size);
    if (newBlock == NULL) {
        return NULL;
    }

    // Zero fill this new block with 0s
    void * startPtr = (void *) (sizeof(struct block) + newBlock);
    bzero(startPtr, size);

    // Copy over data to this new block
    memcpy(startPtr, ptr, currBlock->size);

    // Free the original pointer at the end
    mm_free(ptr);

    return newBlock;
}


void coalesceBlock(struct block * currBlock) {

    if (currBlock->previous && currBlock->previous->free == 1) {
        currBlock->previous->next = currBlock->next;
        currBlock->previous->size += currBlock->size + sizeof(struct block);

        if (currBlock->next) {
            currBlock->next->previous = currBlock->previous;
        }

        struct block * previousBlock = currBlock->next;
        void * ptr = previousBlock + sizeof(struct block);
        bzero(ptr, previousBlock->size);
    }

    
}


void mm_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    int val = checkValidMallocPointer(ptr);
    if (val == 0) {
        return;
    }

    // Get the current block and set it all equal to 0s
    struct block * currBlock = (struct block *) (ptr - sizeof(struct block));
    currBlock->free = 1; 
    bzero(ptr, currBlock->size);

    // Get the next block for coalescing
    struct block * nextBlock = currBlock->next;

    // Check the previous block and the next block 
    struct block * previousBlock = currBlock->previous;
    if (previousBlock != NULL && previousBlock->free == 1) {
        coalesceBlock(currBlock);
    }
    if (nextBlock != NULL && nextBlock->free == 1) {
        coalesceBlock(nextBlock); 
    }

}