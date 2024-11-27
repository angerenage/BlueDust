#pragma once

#include <stddef.h>

//__attribute__((__noreturn__))
void abort(void);

void* malloc(size_t size);
void free(void *ptr);