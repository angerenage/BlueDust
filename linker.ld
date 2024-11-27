ENTRY(_start)

SECTIONS
{
	/* Start address for kernel */
	. = 0xFFFFFFFF80000000;

	/* Code section */
	.text : ALIGN(0x1000) {
		KEEP(*(.text._start)) /* .text._start must be at the beginning */
		*(.text*)
	}

	/* Read-only data section */
	.rodata ALIGN(0x1000) : {
		*(.rodata .rodata.*)
		*(.eh_frame)
	}

	/* Initialized data section */
	.data : {
		*(.data)
	}

	/* Uninitialized data section */
	.bss ALIGN(0x1000) : {
		*(.bss)
		*(COMMON)
	}

	/* Exclude unnecessary sections */
	/DISCARD/ : {
		*(.note.gnu.property)
		*(.comment)
	}
}
