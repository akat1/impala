LIBRARY= libarch
CFLAGS=-D__KERNEL ${_K_FLAGS} 
X86_PREFIX=../x86

SRCS=\
	arch/asmstub.s\
	arch/bootstrap.s\
	arch/init.c\
	arch/interrupt.c\
	arch/isr.s\
	arch/memory.c\
	arch/thread.c\
	${X86_PREFIX}/dev/bus_isa.c\
	${X86_PREFIX}/dev/bus_pci.c\
	${X86_PREFIX}/dev/i8254.c\
	${X86_PREFIX}/dev/i8259a.c\
	${X86_PREFIX}/dev/pckbd.c\
	${X86_PREFIX}/dev/video.c\

include ${IMPALA_MK}/lib.mk
