#pragma once

#include <stddef.h>

#define PAGE_SIZE 4096

typedef struct heap_info {
	void *base;
	size_t num_pages;
} heap_info_t;