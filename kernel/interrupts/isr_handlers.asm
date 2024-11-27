section .text

%macro PUSHALL 0
	push rbx
	push rcx
	push rdx
	push rbp
	push rsi
	push rdi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
%endmacro

%macro POPALL 0
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdi
	pop rsi
	pop rbp
	pop rdx
	pop rcx
	pop rbx
%endmacro

extern divide_by_zero_isr

global divide_by_zero_isr_handler
divide_by_zero_isr_handler:
	pop rdi                               ; address of the instruction causing the exception
	call divide_by_zero_isr
	iretq

extern invalid_opcode_isr

global invalid_opcode_isr_handler
invalid_opcode_isr_handler:
	pop rdi                               ; address of the instruction causing the exception
	call invalid_opcode_isr
	iretq

extern double_fault_isr

global double_fault_isr_handler
double_fault_isr_handler:
	pop rdi                               ; error code
	pop rsi                               ; address of the instruction causing the exception
	call double_fault_isr
	iretq

extern general_protection_fault_isr

global general_protection_fault_isr_handler
general_protection_fault_isr_handler:
	pop rsi                               ; error code
	pop rdi                               ; address of the instruction causing the exception
	call general_protection_fault_isr
	iretq

extern page_fault_isr

global page_fault_isr_handler
page_fault_isr_handler:
	pop rdx                               ; error code
	pop rdi                               ; origin address
	mov rsi, cr2                          ; faulting address
    call page_fault_isr
	iretq

extern keyboard_isr

global keyboard_isr_handler
keyboard_isr_handler:
	push rax
	PUSHALL

	in al, 0x60                           ; read from the keyboard's data buffer
	mov dil, al                           ; move the value to dil (first parameter)
	call keyboard_isr

	mov al, 0x20
	out 0x20, al                          ; send an EOI signal to the PIC

	POPALL
	pop rax
	iretq

extern syscall_isr

global syscall_isr_handler
syscall_isr_handler:
	PUSHALL

	call syscall_isr

	POPALL
	iretq