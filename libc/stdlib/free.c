#include <stdlib.h>

#include "allocator.h"

#include <kernel/kabort.h>

void free(void* ptr) {
	if (!ptr) return;

	memory_block_header_t* block = (memory_block_header_t*)((uint8_t*)ptr - sizeof(memory_block_header_t));

	if (block->magic != MAGIC_ALLOCATED) {
		kabort("Error: Invalid pointer or double free");
		return;
	}

	block->magic = MAGIC_FREE;

	if (block->next && block->next->magic == MAGIC_FREE) {
		block->size += block->next->size;
		block->next = block->next->next;
		if (block->next) {
			block->next->prev = block;
		}
	}

	if (block->prev && block->prev->magic == MAGIC_FREE) {
		block->prev->size += block->size;
		block->prev->next = block->next;
		if (block->next) {
			block->next->prev = block->prev;
		}
		else {
			free_list_tail = block->prev;
		}
	}
}