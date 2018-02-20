/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE"
 * If we meet some day, and you think this stuff is worth it, you can buy us 
 * a beer in return. - AUTHORS
 * ----------------------------------------------------------------------------
 */

#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/syscall.h>
#include <sys/vfs.h>
#include <sys/vm.h>

typedef struct unmount_args unmount_args_t;
struct unmount_args {
    char *path;
};

int sc_unmount(thread_t *, syscall_result_t *, unmount_args_t *);

int
sc_unmount(thread_t *t, syscall_result_t *r, unmount_args_t *args)
{
    char path[PATH_MAX+1];
    int error = 0;
    vnode_t *node;

    if ((error = copyinstr(path, args->path, sizeof(path)))) {
        return error;
    }

    KASSERT(t->thr_proc->p_rootdir != NULL);

    if ((error = vfs_lookup(t->thr_proc->p_curdir, &node, path, t, 
      LKP_NORMAL))) {
        return error;
    }

    vfs_unmount(node);
    vrele(node);
    return error;
}
