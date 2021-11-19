#include <platform.h>

OUTPUT_FORMAT(elf64-littleriscv)

ENTRY(_boot)

STACK_SIZE = 1M;

MEMORY {
    RAM (rwx)    : ORIGIN = MEM_BASE, LENGTH = MEM_SIZE    
}

SECTIONS {

	.boot : {
		*(.boot)
	} 
	__text_begin = .;
	.text :  {
		*(.text)
	}

	.rodata :  {
		*(.rodata)
	}

    .test_table :  {
        _test_table = .;
		*(.test_table)
        _test_table_end = .;
	}
    _test_table_size = (_test_table_end - _test_table) / 8;
	__text_end = .;

	__data_begin = .;
	. = ALIGN(0x1000);
    .tohost : { *(.tohost) }

	.data : {
		*(.data)
	}

	.bss (NOLOAD) :  {	
		__bss_start = .;
		*(.bss) 
		*(.sbss) 
		*(COMMON)	
		__bss_end = .;
	} 

	. = MEM_BASE + 0x1ff000;
    
    . = ALIGN(16) + STACK_SIZE;
    __stack_top = .;
    _end = .;
}
