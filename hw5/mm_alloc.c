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

static struct block head;



static void checkValidMallocPointer(void * ptr) {
	struct block * b = head.next;
	while (b != NULL) {
		void * start = (void *) b + sizeof(*b);
		if (start == ptr) {
			return;
		}
		b = b->next;
	}
}

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



void* mm_malloc(size_t size)
{
  //TODO: Implement malloc
  if (size == 0) {
  	return NULL;
  }

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

  void * beginningOfMemory = (void *) freeBlock + sizeof(struct block);

  if (beginningOfMemory) return beginningOfMemory;

  struct block * tail = (struct block *) sbrk(0);
  sbrk(sizeof(struct block) + size);

  tail->size = size; 
  tail->free = 0; 

  static struct block * tailEnd = &head;
  tailEnd->next = tail;
  tail->previous = tailEnd;
  tail->next = NULL;
  tailEnd = tail;

  void * ptr = (void *) (sizeof(struct block) + tail);
  bzero(ptr, size);

  return ptr;
}

void* mm_realloc(void* ptr, size_t size)
{
  //TODO: Implement realloc

  return NULL;
}

void mm_free(void* ptr)
{
  //TODO: Implement free

  return NULL;
}
