ENTRY (kernel_entrypoint)

OUTPUT(elf64-x86-64)
SECTIONS{
    . = KERNEL_BOOTSTRAP;
    .bootstrap :
        AT ( KERNEL_BOOTSTRAP ) {
        *(.multiboot_header)
        arch/bootstrap.o(.bootstrap)
        arch/bootstrap.o(*)
    }
    . = KERNEL_START + SIZEOF(.bootstrap);
    kernel_start = .;
    .text :
        AT ( ADDR(.bootstrap) + SIZEOF(.bootstrap) )
    {
        *(.text)
    }
    kernel_text_end = .;
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
    kernel_end = .;
}

