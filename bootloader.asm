%define PAGE_PRESENT    (1 << 0)
%define PAGE_WRITE      (1 << 1)

%define CODE_SEG 0x08
%define DATA_SEG 0x10

%ifndef KERNEL_SIZE
	%error "KERNEL_SIZE is not defined. Ensure the build script passes this value."
%endif

%ifndef TOTAL_SIZE
	%error "TOTAL_SIZE is not defined. Ensure the build script passes this value."
%endif

%ifndef PML4_TABLE_BASE
	%error "PML4_TABLE_BASE is not defined. Ensure the build script passes this value."
%endif

%define KERNEL_PAGES ((KERNEL_SIZE + 4095) / 4096)
%define KERNEL_SECTORS ((KERNEL_SIZE + 511) / 512)
%define TOTAL_SECTORS ((TOTAL_SIZE + 511) / 512)

%define KERNEL_ADDRESS_P 0x8000
%define KERNEL_ADDRESS_V 0xFFFFFFFF80000000
%define INFOS_ADDRESS_P 0x1000
%define INFOS_ADDRESS_V 0xFFFFFFFFF7F00000

[BITS 16]
[ORG 0x7C00] ; Bootloader loaded at 0x7C00

start:
	jmp 0x0000:.main ; Jump to main

.main:
	cli ; Disable interrupts

	; Reset segment registers
	xor ax, ax
	mov ss, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	mov sp, start ; Initialize stack pointer

	; Enable A20 line
	in al, 0x92
	or al, 0x02
	out 0x92, al

	; Load kernel and paging structures
	mov bx, KERNEL_ADDRESS_P
	mov dh, 0x00
	mov dl, 0x80 ; Disk number (hard drive)
	mov ch, 0x00
	mov cl, 0x02 ; Start from sector 2
	mov ah, 0x02 ; Function: read sectors
	mov al, TOTAL_SECTORS
	int 0x13
	jc disk_error

	; Initialize ES for kernel info storage
	mov ax, INFOS_ADDRESS_P >> 4
	mov es, ax
	xor di, di ; Offset starts at 0

	; Retrieve memory map using INT 0x15 (E820h)
	mov bx, 0 ; Continuation value
get_memory_map_entry:
	mov eax, 0xE820
	mov edx, 0x534D4150 ; Signature 'SMAP'
	mov ecx, 24 ; Structure size
	int 0x15
	jc mem_map_error ; Jump to error handler if call fails

	inc si ; Increment memory segment counter
	test bx, bx
	jz end_memory_map

	add di, 24 ; Move to the next entry
	jmp get_memory_map_entry

end_memory_map:
	push si ; Save the number of segments

switch_to_long_mode:
	; Disable IRQs by masking all interrupts on PIC
	mov al, 0xFF
	out 0xA1, al
	out 0x21, al

	lidt [IDT]

	; Enable Physical Address Extension (PAE) and Page Global Enable (PGE)
	mov eax, 10100000b
	mov cr4, eax

	; Set up the PML4 table address for paging
	mov edx, PML4_TABLE_BASE
	mov cr3, edx

	; Enable Long Mode by modifying the EFER MSR
	mov ecx, 0xC0000080
	rdmsr
	or eax, 0x00000100 ; Enable Long Mode (LME)
	wrmsr

	; Enable Paging (PG) and Protected Mode (PE)
	mov ebx, cr0
	or ebx, 0x80000001
	mov cr0, ebx

	lgdt [GDT.descriptor]

	; Perform a far jump to Long Mode
	jmp CODE_SEG:long_mode_start

disk_error:
	mov si, disk_message
	jmp print_string

mem_map_error:
	mov si, mem_message
	jmp print_string

print_string:
	lodsb ; Load the next byte from SI into AL
	or al, al
	jz error
	mov ah, 0x0E ; BIOS function to print character
	int 0x10
	jmp print_string

error:
	jmp error ; Infinite loop on error

; -------------------------------
; Empty IDT (Interrupt Descriptor Table)
; -------------------------------
ALIGN 4
IDT:
	dw 0
	dd 0

; -------------------------------
; GDT (Global Descriptor Table)
; -------------------------------
GDT:
	dq 0x0000000000000000 ; Null descriptor
	dq 0x00209A0000000000 ; Code segment descriptor
	dq 0x0000920000000000 ; Data segment descriptor

ALIGN 4
.descriptor:
	dw $ - GDT - 1
	dd GDT

[BITS 64]
long_mode_start:
	; Save kernel info
	mov rdi, INFOS_ADDRESS_V + 0x300
	pop si
	mov DWORD [rdi], esi ; Number of memory segments
	mov QWORD [rdi + 4], KERNEL_ADDRESS_P
	mov QWORD [rdi + 12], KERNEL_SIZE
	mov QWORD [rdi + 20], PML4_TABLE_BASE

	; Jump to kernel entry point
	mov rax, KERNEL_ADDRESS_V
	jmp rax

; -------------------------------
; Strings for error messages
; -------------------------------
mem_message db 'Error reading memory map', 0
disk_message db 'Error reading disk', 0

; -------------------------------
; Boot signature and padding
; -------------------------------
times 510-($-$$) db 0
dw 0xAA55
