#include "memory.h"

#include <stdio.h>
#include <stdlib.h>

#include <kernel/memory/heap.h>

#include <kernel/kabort.h>

#include "paging.h"

static boot_info_t boot_info = {0};

void init_memory(boot_info_t *info) {
	boot_info = *info;

	printf("Kernel size: %llu bytes (%d pages)\n", boot_info.kernel_size, (((boot_info.kernel_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))) / PAGE_SIZE);
	printf("PML4 table address: 0x%llx\n\n", boot_info.pml4_table);

	printf("Memory map entries: %d\n", boot_info.memory_map_entries);
	
	size_t total_memory = 0, available_memory = 0;
	for (unsigned int i = 0; i < boot_info.memory_map_entries; i++) {
		uint64_t base = boot_info.memory_map[i].base_addr;
		uint64_t length = boot_info.memory_map[i].length;
		uint32_t type = boot_info.memory_map[i].type;

		total_memory += length;
		if (type == 1) available_memory += length;

		printf("\tSegment %d (base=0x%llx, length=0x%llx, type=%d)\n", i, base, length, type);
	}

	printf("Available memory: %zu MB / %zu MB\n\n", available_memory / 1024 / 1024, total_memory / 1024 / 1024);
}

int map_physical_address(uint64_t physical_address, uint64_t virtual_address, uint64_t flags) {
	return map_page((uint64_t*)boot_info.pml4_table, physical_address, virtual_address, flags);
}