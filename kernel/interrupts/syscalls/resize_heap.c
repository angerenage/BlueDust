#include "syscall_isr.h"

#include <kernel/kabort.h>

#include <stdio.h>

#include "../../memory/memory.h"
#include "../../memory/allocator.h"

int resize_heap(heap_info_t *heap_info, int pages_delta) {
	if (heap_info == NULL) return 1;
	if (pages_delta == 0) return 0;

	if (heap_info->base == NULL) {
		heap_info->base = (void*)0xFFFFFFFFF7F00000;
		heap_info->num_pages = 1;
		pages_delta--;
	}

	if (pages_delta != 0) {
		if (free_pages_list_head == NULL && pages_delta > 0) {
			if (heap_info->num_pages + pages_delta > 7) return 1;

			for (int i = 0; i < pages_delta; i++) {
				uint64_t offset = 0x1000 * (heap_info->num_pages + i);
				if (map_physical_address(0x1000 + offset, ((uint64_t)heap_info->base) + offset, 0x3)) return 1;
			}
		}
		else if (free_pages_list_head != NULL) {
			// TODO: Implement heap resizing with free list
		}
		else {
			kabort("Cannot resize heap");
		}

		heap_info->num_pages += pages_delta;
	}

	return 0;
}