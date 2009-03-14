LIBS=-larch -lkutil 
KERNEL=impala
LIBDEPS=../arch/${IMPALA_ARCH}/libarch.a ../libkutil/libkutil.a

SRCS=\
	clock.c\
	kprintf.c\
	kthread.c\
	main.c\
	sched.c\
	syscall.c\
	thread.c\
	utils.c\
	sc/exit.c\
	sc/write.c\




include ${IMPALA_MK}/kernel.mk 
