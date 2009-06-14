OUTPUT_FORMAT("a.out-i386")
OUTPUT_ARCH(i386)
ENTRY(___start)
SEARCH_DIR("../../lib/crt") 
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
