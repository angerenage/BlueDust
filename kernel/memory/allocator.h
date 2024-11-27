#pragma once

#include <stdint.h>

typedef struct memory_block {
	uint64_t start_address;
	uint64_t size_in_pages;
	struct memory_block* next;
} memory_block_t;

extern memory_block_t* free_pages_list_head;

uint64_t allocate_pages(uint32_t num_pages);
void free_pages(uint64_t start_address, uint32_t num_pages);