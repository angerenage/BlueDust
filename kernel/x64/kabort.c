#include <kernel/kabort.h>

#include <kernel/tty.h>

#include <string.h>
#include <stdio.h>

//__attribute__((__noreturn__))
void kabort(const char* message) {
	terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE));
	terminal_clear();
	terminal_writeat("Kernel panic: kabort()", VGA_WIDTH / 2 - 21 / 2, VGA_HEIGHT / 2 - 1);
	terminal_writeat(message, VGA_WIDTH / 2 - strlen(message) / 2, VGA_HEIGHT / 2);

	//printf("Kernel panic: kabort()\n");
	//printf("%s\n", message);

	asm volatile("hlt");

	while (1);
	__builtin_unreachable();
}