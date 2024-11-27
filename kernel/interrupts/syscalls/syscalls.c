#include <kernel/interrupt/syscalls.h>
#include <stdint.h>

#include <stdio.h>

#include "../IDT.h"

#include "syscall_isr.h"

extern void syscall_isr_handler();

void init_syscalls() {
	set_idt_entry(0x80, syscall_isr_handler, 0xEE);
}

int syscall_isr(int syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
    switch (syscall_number) {
        case 0: // Test syscall
            printf("Syscall 0 called with arg1: %llu\n", arg1);
            break;
        case 1: // resize_heap
            return resize_heap((heap_info_t*)arg1, (int)arg2);
        default:
            printf("Unknown syscall: %llu\n", syscall_number);
    }

	return 0;
}

static int syscall(int syscall_number, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
	int ret;
	asm volatile(
		"mov %1, %%rdi\n"
		"mov %2, %%rsi\n"
		"mov %3, %%rdx\n"
		"mov %4, %%rcx\n"
		"int $0x80\n"
		"mov %%eax, %0\n"
		: "=a"(ret)
		: "g"(syscall_number), "g"(arg1), "g"(arg2), "g"(arg3)
		: "rdi", "rsi", "rdx", "rcx"
	);
	return ret;
}

int sys_resize_heap(heap_info_t *heap_info, int pages_delta) {
	return syscall(1, (uint64_t)heap_info, (uint64_t)pages_delta, 0);
}