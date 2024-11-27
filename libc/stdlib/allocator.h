#pragma once

#include <kernel/memory/heap.h>

#include <stdint.h>
#include <stddef.h>

struct memory_block_header {
	uint32_t magic;
	uint32_t flags;
	uint64_t size;

	struct memory_block_header* prev;
	struct memory_block_header* next;
} __attribute__((packed));
typedef struct memory_block_header memory_block_header_t;

#define MAGIC_ALLOCATED 0x12345678
#define MAGIC_FREE 0x87654321
#define ALIGNMENT _Alignof(max_align_t)

extern heap_info_t heap_info;

extern memory_block_header_t* free_list_head;
extern memory_block_header_t* free_list_tail;