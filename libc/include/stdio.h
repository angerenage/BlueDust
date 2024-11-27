#pragma once

#include <stdarg.h>

#define EOF (-1)

void sprintf(char* buffer, const char* __restrict format, ...);
void printf(const char* __restrict format, ...);
void putchar(char);
void puts(const char*);