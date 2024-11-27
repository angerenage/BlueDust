#pragma once

#include <stddef.h>
#include <stdint.h>

#include "../x64/vga.h"

void init_terminal();
void terminal_clear();
void terminal_setcolor(uint8_t color);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
void terminal_writeat(const char* data, uint16_t x, uint16_t y);