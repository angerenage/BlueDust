#include "IDT.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <kernel/tty.h>
#include <kernel/io.h>
#include <kernel/kabort.h>
#include <kernel/interrupt/syscalls.h>

struct idt_entry {
	uint16_t offset_low;
	uint16_t selector;
	uint8_t  ist;
	uint8_t  type_attr;
	uint16_t offset_mid;
	uint32_t offset_high;
	uint32_t zero;
} __attribute__((packed));

struct idt_entry idt[256];

struct idtr {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed));

struct idtr idt_descriptor;

extern void divide_by_zero_isr_handler();
extern void invalid_opcode_isr_handler();
extern void double_fault_isr_handler();
extern void general_protection_fault_isr_handler();
extern void page_fault_isr_handler();

extern void keyboard_isr_handler();

void divide_by_zero_isr(uint64_t address) {
	char message[100];
	sprintf(message, "Division by zero at 0x%llx", address);
	kabort(message);
}

void invalid_opcode_isr(uint64_t address) {
	char message[100];
	sprintf(message, "Invalid opcode at 0x%llx", address);
	kabort(message);
}

void double_fault_isr(uint64_t address, uint64_t error_code) {
	char message[300];
	sprintf(message, "Double fault (saved isp 0x%llx, error code 0x%llx)", address, error_code);
	kabort(message);
}

void general_protection_fault_isr(uint64_t address, uint64_t error_code) {
	char message[300];
	sprintf(message, "General protection fault at 0x%llx (error code 0x%llx)", address, error_code);
	kabort(message);
}

void page_fault_isr(uint64_t origin, uint64_t faulting_address, uint64_t error_code) {
	bool present = error_code & 0x1;
	bool rw = error_code & 0x2;
	bool user = error_code & 0x4;
	bool reserved = error_code & 0x8;
	bool execute = error_code & 0x10;

	char message[300];
	sprintf(message, "Page Fault at 0x%llx accessing 0x%llx [%c%c%c%c%c]", origin, faulting_address, present ? 'P' : '-', rw ? 'W' : '-', user ? 'U' : '-', reserved ? 'R' : '-', execute ? 'E' : '-');
	kabort(message);
}

void keyboard_isr(uint8_t scancode) {
	printf("Scancode: 0x%x\n", scancode);
}

static void pic_remap() {
	// ICW1: Initialize the PIC
	outb(0x20, 0x11); // Master PIC
	outb(0xA0, 0x11); // Slave PIC

	// ICW2: Set the interrupt vector offset
	outb(0x21, 0x20); // Master PIC starts at 0x20
	outb(0xA1, 0x28); // Slave PIC starts at 0x28

	// ICW3: Tell the master PIC that there is a slave PIC at IRQ2 (0000 0100)
	outb(0x21, 0x04);
	outb(0xA1, 0x02);

	// ICW4: Set the mode (8086/88 mode)
	outb(0x21, 0x01);
	outb(0xA1, 0x01);

	// Mask all interrupts
	outb(0x21, 0xFF);
	outb(0xA1, 0xFF);
}

static void unmask_irq(int irq) {
	uint16_t port;
	uint8_t value;

	if (irq < 8) {
		port = 0x21;
	}
	else {
		port = 0xA1;
		irq -= 8;
	}

	value = inb(port) & ~(1 << irq);
	outb(port, value);
}

void init_idt() {
	pic_remap();

	asm volatile("sti");

	// Unmask the keyboard interrupt
	unmask_irq(1);

	set_idt_entry(0x0, divide_by_zero_isr_handler, 0x8E);
	set_idt_entry(0x6, invalid_opcode_isr_handler, 0x8E);
	set_idt_entry(0x8, double_fault_isr_handler, 0x8E);
	set_idt_entry(0xD, general_protection_fault_isr_handler, 0x8E);
	set_idt_entry(0xE, page_fault_isr_handler, 0x8E);

	init_syscalls();

	set_idt_entry(0x21, keyboard_isr_handler, 0x8E);

	idt_descriptor.limit = sizeof(idt) - 1;
	idt_descriptor.base = (uint64_t)&idt;
	asm volatile("lidt %0" : : "m"(idt_descriptor));
}

void set_idt_entry(int vector, void (*handler)(), uint8_t type_attr) {
	uint64_t handler_address = (uint64_t)handler;
	idt[vector].offset_low = handler_address & 0xFFFF;
	idt[vector].selector = 0x08;
	idt[vector].ist = 0;
	idt[vector].type_attr = type_attr;
	idt[vector].offset_mid = (handler_address >> 16) & 0xFFFF;
	idt[vector].offset_high = (handler_address >> 32) & 0xFFFFFFFF;
	idt[vector].zero = 0;
}