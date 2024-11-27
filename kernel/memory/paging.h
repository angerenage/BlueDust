#pragma once

#include <stdint.h>

#define PAGE_PRESENT 0x1

int map_page(uint64_t* const pml4_table, uint64_t physical_address, uint64_t virtual_address, uint64_t flags);
int unmap_page(uint64_t* const pml4_table, uint64_t virtual_address);