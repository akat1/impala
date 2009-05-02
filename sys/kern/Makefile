LIBS=-larch -lkutil -ldev -lfs
KERNEL=impala
KERNEL_MAIN= main.c
LIBDEPS=\
	../arch/${IMPALA_ARCH}/libarch.a\
	../libkutil/libkutil.a\
	../dev/libdev.a\
	../fs/libfs.a\

SRCS=\
	kern_clock.c\
	kern_bio.c\
	kern_console.c\
	kern_dev.c\
	kern_file.c\
	kern_kthread.c\
	kern_main.c\
	kern_mem.c\
	kern_sched.c\
	kern_syscall.c\
	kern_thread.c\
	kern_uio.c\
	kern_utils.c\
	vfs.c\
	vfs_node.c\
	vm.c\
	vm_lpool.c\
	vm_segment.c\
	vm_trap.c\
	sc/exit.c\
	sc/write.c\
	tmp_rootimage.c\




include ${IMPALA_MK}/kernel.mk 
