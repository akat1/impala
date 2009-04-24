LIBS=-larch -lkutil -ldev
KERNEL=impala
KERNEL_MAIN= main.c
LIBDEPS=\
	../arch/${IMPALA_ARCH}/libarch.a\
	../libkutil/libkutil.a\
	../dev/libdev.a\

SRCS=\
	clock.c\
	kprintf.c\
	kern_bio.c\
	kern_file.c\
	kern_dev.c\
	kern_mem.c\
	kern_uio.c\
	kthread.c\
	main.c\
	sched.c\
	syscall.c\
	thread.c\
	utils.c\
	vm.c\
	vm_lpool.c\
	vm_segment.c\
	vm_trap.c\
	sc/exit.c\
	sc/write.c\




include ${IMPALA_MK}/kernel.mk 
