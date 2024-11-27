#pragma once

#include <stdint.h>

#include <kernel/memory/heap.h>

void init_syscalls();

int sys_resize_heap(heap_info_t *heap_info, int pages_delta);