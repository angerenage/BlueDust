#include <stdio.h>
#include <stdlib.h>

#include <kernel/tty.h>

__attribute__((__noreturn__))
void abort() {
    asm volatile("hlt");

	while (1);
	__builtin_unreachable();
}