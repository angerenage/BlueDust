#include <stdbool.h>
#include <stdio.h>

#include <kernel/tty.h>

#include <stdlib.h>

#include "memory/memory.h"
#include "interrupts/IDT.h"

#define BOOT_INFO_ADDR 0xFFFFFFFFF7F00000

void main() {
	init_terminal();

	init_idt();

	init_memory((boot_info_t*)BOOT_INFO_ADDR);

	// Test malloc
	/*printf("allocating until OoM\n");
	void *addr = NULL;
	int i = 0;
	for (int i = 0; true; i++) {
		addr = malloc(4096);
		if (!addr) {
			printf("OoM after %d malloc\n", i + 1);
			break;
		}
	}*/

	// Test syscall
	//printf("Calling syscall 0\n");
	//asm volatile("int $0x80" : : "a"(0));

	// Test page fault
	//int *ptr = (int*)0x31;
	//printf("Trying to access NULL pointer: %d\n", *ptr);

	// Test division by zero
	//volatile int a = 3;
	//int b = 5 / (a - 3);
	//printf("5.0f / 0.0f = %f\n", b);

	// Test invalid opcode
	//asm volatile("ud2");

	// Test general protection fault
	//volatile int *invalid_ptr = (int*)0x0000800000000000;
	//printf("Trying to access invalid memory address: %d\n", *invalid_ptr);
}