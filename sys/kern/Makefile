LIBS=-larch -lkutil 
KERNEL=impala
KERNEL_MAIN= main.c
LIBDEPS=../arch/${IMPALA_ARCH}/libarch.a ../libkutil/libkutil.a

SRCS=\
	clock.c\
	kprintf.c\
	kern_mem.c\
	kthread.c\
	main.c\
	sched.c\
	syscall.c\
	thread.c\
	utils.c\
	vm.c\
	vm_segment.c\
	vm_trap.c\
	sc/exit.c\
	sc/write.c\




include ${IMPALA_MK}/kernel.mk 
