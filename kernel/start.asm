section .text._start

extern main

%define STACK_ADDR 0xFFFFFFFF7FFFD000

global _start
_start:
	; Set up the stack pointer
	mov rsp, STACK_ADDR + 0x1000 - 1

	; Load the Global Descriptor Table (GDT)
	lgdt [GDT.descriptor]

	; Load segment selectors for data and stack
	mov ax, 0x10            ; Data segment selector
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	; Enable Page Global Enable (PGE) in CR4
	mov rax, cr4
	or rax, 0x200           ; Set the PGE bit
	mov cr4, rax

	; Call the kernel's main function
	call main

_end:
	; Halt the CPU in an infinite loop
	hlt
	jmp _end

section .rodata

ALIGN 8
GDT:
	dq 0x0000000000000000   ; Null segment
	dq 0x00af9a000000ffff   ; Code segment
	dq 0x00af92000000ffff   ; Data segment

.descriptor:
	dw $ - GDT - 1          ; GDT limit
	dq GDT                  ; GDT base
