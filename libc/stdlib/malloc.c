#include <stdlib.h>

#include "allocator.h"

#include <kernel/kabort.h>
#include <kernel/interrupt/syscalls.h>

#include <stdio.h>

heap_info_t heap_info = {0};

memory_block_header_t* free_list_head = NULL;
memory_block_header_t* free_list_tail = NULL;

static void init_allocator() {
	if (sys_resize_heap(&heap_info, 1)) {
		kabort("Error: Failed to initialize allocator");
	}

	free_list_head = (memory_block_header_t*)heap_info.base;
	free_list_head->magic = MAGIC_FREE;
	free_list_head->flags = 0;
	free_list_head->size = heap_info.num_pages * PAGE_SIZE - sizeof(memory_block_header_t);
	free_list_head->prev = NULL;
	free_list_head->next = NULL;

	free_list_tail = free_list_head;
}

void* malloc(size_t size) {
	if (size == 0) return NULL;
	if (!free_list_head || !free_list_tail) init_allocator();

	size_t aligned_size = (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
	size_t total_size = aligned_size + sizeof(memory_block_header_t);

	memory_block_header_t* curr = free_list_head;
	while (curr) {
		if (curr->magic == MAGIC_FREE && curr->size >= total_size) {
			size_t remaining_size = curr->size - total_size;

			if (remaining_size > sizeof(memory_block_header_t)) {
				memory_block_header_t* new_block = (memory_block_header_t*)((uint8_t*)curr + total_size);
				new_block->magic = MAGIC_FREE;
				new_block->flags = 0;
				new_block->size = remaining_size;
				new_block->prev = curr;
				new_block->next = curr->next;

				if (curr->next) {
					curr->next->prev = new_block;
				}

				curr->next = new_block;
				curr->size = total_size;

				if (curr == free_list_tail) {
					free_list_tail = new_block;
				}
			}

			curr->magic = MAGIC_ALLOCATED;
			return (void*)((uint8_t*)curr + sizeof(memory_block_header_t));
		}

		curr = curr->next;
	}

	if (sys_resize_heap(&heap_info, 1)) {
		printf("Error: Failed to allocate %zu bytes\n", size);
		return NULL;
	}

	if (free_list_tail->magic == MAGIC_FREE) {
		free_list_tail->size += PAGE_SIZE;
	}
	else {
		memory_block_header_t* new_block = (memory_block_header_t*)((uint8_t*)free_list_tail + sizeof(memory_block_header_t) + free_list_tail->size);
		new_block->magic = MAGIC_FREE;
		new_block->flags = 0;
		new_block->size = PAGE_SIZE - sizeof(memory_block_header_t);
		new_block->prev = free_list_tail;
		new_block->next = NULL;

		free_list_tail->next = new_block;
		free_list_tail = new_block;
	}

	return malloc(size);
}