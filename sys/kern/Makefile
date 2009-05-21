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
	kern_exec.c\
	kern_file.c\
	kern_kthread.c\
	kern_main.c\
	kern_mem.c\
	kern_proc.c\
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
	vm_space.c\
	vm_trap.c\
	sc/close.c\
        sc/execve.c\
        sc/exit.c\
        sc/fork.c\
        sc/getpid.c\
        sc/getppid.c\
        sc/getuid.c\
        sc/kill.c\
        sc/lseek.c\
        sc/nanosleep.c\
        sc/open.c\
        sc/pause.c\
        sc/read.c\
        sc/setuid.c\
        sc/sigaction.c\
        sc/wait.c\
        sc/waitpid.c\
        sc/write.c\
	tmp_rootimage.c\




include ../../mk/kernel.mk 
