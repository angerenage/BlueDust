#include "paging.h"

#include <stdio.h>

#include "allocator.h"

int map_page(uint64_t* const pml4_table, uint64_t physical_address, uint64_t virtual_address, uint64_t flags) {
	uint64_t pml4_index = (virtual_address >> 39) & 0x1FF;
	uint64_t pdpt_index = (virtual_address >> 30) & 0x1FF;
	uint64_t pd_index = (virtual_address >> 21) & 0x1FF;
	uint64_t pt_index = (virtual_address >> 12) & 0x1FF;

	// Get or create the PDPT
	if (!(pml4_table[pml4_index] & PAGE_PRESENT)) {
		uint64_t *new_pdpt = (uint64_t *)allocate_pages(1);
		if (!new_pdpt) return 1;
		pml4_table[pml4_index] = ((uint64_t)new_pdpt & ~0xFFF) | flags | PAGE_PRESENT;
	}
	uint64_t *pdpt = (uint64_t *)(pml4_table[pml4_index] & ~0xFFF);

	// Get or create the PD
	if (!(pdpt[pdpt_index] & PAGE_PRESENT)) {
		uint64_t *new_pd = (uint64_t *)allocate_pages(1);
		if (!new_pd) return 1;
		pdpt[pdpt_index] = ((uint64_t)new_pd & ~0xFFF) | flags | PAGE_PRESENT;
	}
	uint64_t *pd = (uint64_t *)(pdpt[pdpt_index] & ~0xFFF);

	// Get or create the PT
	if (!(pd[pd_index] & PAGE_PRESENT)) {
		uint64_t *new_pt = (uint64_t *)allocate_pages(1);
		if (!new_pt) return 1;
		pd[pd_index] = ((uint64_t)new_pt & ~0xFFF) | flags | PAGE_PRESENT;
	}
	uint64_t *pt = (uint64_t *)(pd[pd_index] & ~0xFFF);

	// Map the page in the PT
	if (pt[pt_index] & PAGE_PRESENT) {
		printf("Page 0x%llx already mapped\n", virtual_address);
		return 1; // Page already mapped
	}
	pt[pt_index] = (physical_address & ~0xFFF) | flags | PAGE_PRESENT;

	return 0;
}

int unmap_page(uint64_t* const pml4_table, uint64_t virtual_address) {
	uint64_t pml4_index = (virtual_address >> 39) & 0x1FF;
	uint64_t pdpt_index = (virtual_address >> 30) & 0x1FF;
	uint64_t pd_index = (virtual_address >> 21) & 0x1FF;
	uint64_t pt_index = (virtual_address >> 12) & 0x1FF;

	if (!(pml4_table[pml4_index] & PAGE_PRESENT)) return 1;
	uint64_t *pdpt = (uint64_t *)(pml4_table[pml4_index] & ~0xFFF);

	if (!(pdpt[pdpt_index] & PAGE_PRESENT)) return 1;
	uint64_t *pd = (uint64_t *)(pdpt[pdpt_index] & ~0xFFF);

	if (!(pd[pd_index] & PAGE_PRESENT)) return 1;
	uint64_t *pt = (uint64_t *)(pd[pd_index] & ~0xFFF);

	if (!(pt[pt_index] & PAGE_PRESENT)) return 1;
	pt[pt_index] = 0;

	return 0;
}