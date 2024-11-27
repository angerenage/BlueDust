#include "allocator.h"

#include <kernel/kabort.h>

#include <stddef.h>

memory_block_t* free_pages_list_head = NULL;

uint64_t allocate_pages(uint32_t num_pages) {
	kabort("allocate_pages not implemented");
}

void free_pages(uint64_t start_address, uint32_t num_pages) {
	kabort("free_pages not implemented");
}
