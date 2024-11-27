#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// E820 memory map entry
struct e820_entry {
	uint64_t base_addr;
	uint64_t length;
	uint32_t type;
	uint32_t attr;
} __attribute__((packed));
typedef struct e820_entry e820_entry_t;

struct boot_info {
	e820_entry_t memory_map[32]; // memory map entries
	uint32_t memory_map_entries; // number of memory map entries
	uint64_t kernel_base;        // kernel base address in physical memory
	uint64_t kernel_size;        // kernel size in bytes
	uint64_t pml4_table;         // PML4 table address in physical memory
} __attribute__((packed));
typedef struct boot_info boot_info_t;

void init_memory(boot_info_t *info);

int map_physical_address(uint64_t physical_address, uint64_t virtual_address, uint64_t flags);