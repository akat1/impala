ENTRY (kernel_entrypoint)
OUTPUT(elf32-i386)
SECTIONS{
    . = 0x00100000;

    .text :{
	arch/bootstrap.o(.text)
        *(.text)
    }

    .rodata ALIGN (0x1000) : {
        *(.rodata)
    }

    .data ALIGN (0x1000) : {
        *(.data)
    }

    .bss  ALIGN(0x1000) : {
        *(COMMON)
        *(.bss)
    }

}

