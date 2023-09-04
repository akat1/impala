OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(__start)
SEARCH_DIR("../../lib/crt") 
INPUT(crt0.o -lgcc -lc)
SECTIONS
{
  . = 0;
  .text :
  {
    crt0.o(.text)
    *(.text)
    _etext = .;
    __etext = .;
    ___etext = .;
  }
  .data ALIGN(0x1000) :
  {
    crt0.o(.data)
    *(.data)
    _edata  =  .;
    __edata  =  .;
    ___edata = .;
  }
  .bss ALIGN(0x1000) :
  {
    __bss_start = .;
   *(.bss)
   *(COMMON)
   . = ALIGN(4);
   _end = . ;
   __end = . ;
   ___end = .;
  }
}
