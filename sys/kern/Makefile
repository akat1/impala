LIBS=-larch -lkutil -ldev -lfs
KERNEL=impala
KERNEL_MAIN= main.c
LIBDEPS=\
    systab.c\
	../arch/${IMPALA_ARCH}/libarch.a\
	../libkutil/libkutil.a\
	../dev/libdev.a\
	../fs/libfs.a\

SRCS=\
	kern_clock.c\
	kern_bio.c\
	kern_console.c\
	kern_ctty.c\
	kern_dev.c\
	kern_exec.c\
	kern_file.c\
	kern_kthread.c\
	kern_main.c\
	kern_mem.c\
	kern_proc.c\
	kern_sched.c\
	kern_syscall.c\
	kern_sysvipc.c\
	kern_thread.c\
	kern_tty.c\
	kern_uio.c\
	kern_utils.c\
	vfs.c\
	vfs_node.c\
	vm.c\
	vm_lpool.c\
	vm_segment.c\
	vm_space.c\
	vm_trap.c\
	tmp_rootimage.c\
	${SC_SRCS}

include Makefile.sc
include ../../mk/kernel.mk 

systab.c: systab.in
	@echo " GEN systab"
	@${AWK} -f gensystab.awk < systab.in
	@${MAKE} build

