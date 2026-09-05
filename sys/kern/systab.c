/* FILE GENERATED AUTOMATICALLY, ALL CHANGES WILL BE OVERWRITTEN  */
/* Generator: gensystab.awk */

#include <sys/types.h>

sc_handler_f sc_access;
sc_handler_f sc_execve;
sc_handler_f sc_exit;
sc_handler_f sc_chmod;
sc_handler_f sc_chown;
sc_handler_f sc_fchmod;
sc_handler_f sc_fcntl;
sc_handler_f sc_fork;
sc_handler_f sc_fstat;
sc_handler_f sc_fsync;
sc_handler_f sc_ftell;
sc_handler_f sc_ftruncate;
sc_handler_f sc_getcwd;
sc_handler_f sc_getegid;
sc_handler_f sc_geteuid;
sc_handler_f sc_getdents;
sc_handler_f sc_getpgid;
sc_handler_f sc_getpid;
sc_handler_f sc_getppid;
sc_handler_f sc_getuid;
sc_handler_f sc_ioctl;
sc_handler_f sc_isatty;
sc_handler_f sc_kill;
sc_handler_f sc_lseek;
sc_handler_f sc_mkdir;
sc_handler_f sc_nanosleep;
sc_handler_f sc_open;
sc_handler_f sc_pause;
sc_handler_f sc_read;
sc_handler_f sc_select;
sc_handler_f sc_setpgid;
sc_handler_f sc_setsid;
sc_handler_f sc_setuid;
sc_handler_f sc_setreuid;
sc_handler_f sc_sigaction;
sc_handler_f sc_wait;
sc_handler_f sc_waitpid;
sc_handler_f sc_write;
sc_handler_f sc_close;
sc_handler_f sc_link;
sc_handler_f sc_unlink;
sc_handler_f sc_pipe;
sc_handler_f sc_stat;
sc_handler_f sc_symlink;
sc_handler_f sc_rmdir;
sc_handler_f sc_dup;
sc_handler_f sc_dup2;
sc_handler_f sc_chdir;
sc_handler_f sc_getgid;
sc_handler_f sc_setregid;
sc_handler_f sc_setgid;
sc_handler_f sc_umask;
sc_handler_f sc_msgget;
sc_handler_f sc_msgctl;
sc_handler_f sc_msgsnd;
sc_handler_f sc_msgrcv;
sc_handler_f sc_thr_create;
sc_handler_f sc_thr_exit;
sc_handler_f sc_thr_cancel;
sc_handler_f sc_thr_getid;
sc_handler_f sc_thr_getarg;
sc_handler_f sc_thr_join;
sc_handler_f sc_thr_mtx_create;
sc_handler_f sc_thr_mtx_destroy;
sc_handler_f sc_thr_mtx_lock;
sc_handler_f sc_thr_mtx_trylock;
sc_handler_f sc_thr_mtx_unlock;
sc_handler_f sc_thr_mtx_wait;
sc_handler_f sc_thr_mtx_wakeup;
sc_handler_f sc_thr_mtx_wakeup_all;
sc_handler_f sc_ttyname;
sc_handler_f sc_sched_yield;
sc_handler_f sc_sbrk;
sc_handler_f sc_signal;
sc_handler_f sc_sigpending;
sc_handler_f sc_sigprocmask;
sc_handler_f sc_sigreturn;
sc_handler_f sc_getprocinfo;
sc_handler_f sc_getmountinfo;
sc_handler_f sc_uname;
sc_handler_f sc_writev;
sc_handler_f sc_gettimeofday;
sc_handler_f sc_nice;
sc_handler_f sc_mount;
sc_handler_f sc_unmount;

static sc_handler_f *syscall_table[] = {
    sc_access,
    sc_execve,
    sc_exit,
    sc_chmod,
    sc_chown,
    sc_fchmod,
    sc_fcntl,
    sc_fork,
    sc_fstat,
    sc_fsync,
    sc_ftell,
    sc_ftruncate,
    sc_getcwd,
    sc_getegid,
    sc_geteuid,
    sc_getdents,
    sc_getpgid,
    sc_getpid,
    sc_getppid,
    sc_getuid,
    sc_ioctl,
    sc_isatty,
    sc_kill,
    sc_lseek,
    sc_mkdir,
    sc_nanosleep,
    sc_open,
    sc_pause,
    sc_read,
    sc_select,
    sc_setpgid,
    sc_setsid,
    sc_setuid,
    sc_setreuid,
    sc_sigaction,
    sc_wait,
    sc_waitpid,
    sc_write,
    sc_close,
    sc_link,
    sc_unlink,
    sc_pipe,
    sc_stat,
    sc_symlink,
    sc_rmdir,
    sc_dup,
    sc_dup2,
    sc_chdir,
    sc_getgid,
    sc_setregid,
    sc_setgid,
    sc_umask,
    sc_msgget,
    sc_msgctl,
    sc_msgsnd,
    sc_msgrcv,
    sc_thr_create,
    sc_thr_exit,
    sc_thr_cancel,
    sc_thr_getid,
    sc_thr_getarg,
    sc_thr_join,
    sc_thr_mtx_create,
    sc_thr_mtx_destroy,
    sc_thr_mtx_lock,
    sc_thr_mtx_trylock,
    sc_thr_mtx_unlock,
    sc_thr_mtx_wait,
    sc_thr_mtx_wakeup,
    sc_thr_mtx_wakeup_all,
    sc_ttyname,
    sc_sched_yield,
    sc_sbrk,
    sc_signal,
    sc_sigpending,
    sc_sigprocmask,
    sc_sigreturn,
    sc_getprocinfo,
    sc_getmountinfo,
    sc_uname,
    sc_writev,
    sc_gettimeofday,
    sc_nice,
    sc_mount,
    sc_unmount,
    NULL
};
