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
	kern_sysvipc.c\
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
	sc/brk.c\
	sc/chdir.c\
	sc/close.c\
	sc/execve.c\
	sc/exit.c\
	sc/fork.c\
	sc/getdents.c\
	sc/getgid.c\
	sc/getpid.c\
	sc/getppid.c\
	sc/getuid.c\
	sc/kill.c\
	sc/lseek.c\
	sc/nanosleep.c\
	sc/open.c\
	sc/pause.c\
	sc/read.c\
	sc/sbrk.c\
	sc/setgid.c\
	sc/setuid.c\
	sc/sigaction.c\
	sc/wait.c\
	sc/waitpid.c\
	sc/write.c\
	sc/link.c\
	sc/unlink.c\
	sc/symlink.c\
	sc/rmdir.c\
	sc/dup.c\
	sc/dup2.c\
	sc/umask.c\
	tmp_rootimage.c\



include ../../mk/kernel.mk 
