#include <kernel/io.h>

uint8_t inb(unsigned short port) {
	uint8_t data;
	asm volatile ("inb %1, %0" : "=g" (data) : "dN" (port));
	return data;
}