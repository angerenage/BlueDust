#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/tty.h>
#include <kernel/io.h>

const size_t VGA_WIDTH = 80;
const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t cursor_x, cursor_y;

static void move_cursor(uint16_t x, uint16_t y) {
	cursor_x = x;
	cursor_y = y;

	uint16_t position = y * VGA_WIDTH + x;
	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t)(position & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t)((position >> 8) & 0xFF));
}

void init_terminal() {
	terminal_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
	terminal_clear();
}

void terminal_clear() {
	terminal_row = 0;
	terminal_column = 0;
	for (size_t i = 0; i < VGA_HEIGHT * VGA_WIDTH; i++) {
		VGA_MEMORY[i] = vga_entry(' ', terminal_color);
	}
	move_cursor(0, 0);
}

void terminal_setcolor(uint8_t color) {
	terminal_color = color;
}

static void terminal_putentryat(unsigned char c, uint8_t color, uint16_t x, uint16_t y) {
	const size_t index = y * VGA_WIDTH + x;
	VGA_MEMORY[index] = vga_entry(c, color);
}

void terminal_scroll() {
	for (size_t y = 1; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			VGA_MEMORY[(y - 1) * VGA_WIDTH + x] = VGA_MEMORY[y * VGA_WIDTH + x];
		}
	}

	for (size_t x = 0; x < VGA_WIDTH; x++) {
		VGA_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
	}

	if (terminal_row > 0) {
		terminal_row--;
	}
	move_cursor(terminal_column, terminal_row);
}

void terminal_putchar(char c) {
	switch (c) {
		case '\n':
			terminal_column = 0;
			terminal_row++;
			break;
		case '\t':
			terminal_column = (terminal_column + 4) & ~(4 - 1); // Align to 4
			if (terminal_column >= VGA_WIDTH) {
				terminal_column = 0;
				terminal_row++;
			}
			break;
		default:
			terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
			if (++terminal_column == VGA_WIDTH) {
				terminal_column = 0;
				terminal_row++;
			}
			break;
	}

	if (terminal_row == VGA_HEIGHT) {
		terminal_scroll();
	}
	move_cursor(terminal_column, terminal_row);
}

void terminal_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++) terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
	while (*data) terminal_putchar(*data++);
}

void terminal_writeat(const char* data, uint16_t x, uint16_t y) {
	for (size_t i = 0; data[i]; i++) {
		terminal_putentryat(data[i], terminal_color, x + i, y);
		move_cursor(x + i, y);
	}
}