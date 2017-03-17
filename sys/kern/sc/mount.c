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
#include <sys/vm.h>

typedef struct mount_args mount_args_t;
struct mount_args {
    char *type;
    char *target;
    int flags;
    void *data;
};

int sc_mount(thread_t *, syscall_result_t *, mount_args_t *);

int
sc_mount(thread_t *t, syscall_result_t *r, mount_args_t *args)
{
    char type[128];
    char target[PATH_MAX+1], device[PATH_MAX+1];
    int error = 0;
    vnode_t *target_node, *device_node;

    if ((error = copyinstr(type, args->type, sizeof(type))))
        return error;

    if ((error = copyinstr(target, args->target, sizeof(target))))
        return error;

    /* for now we assume that only device path could be as data arg */
    if ((error = copyinstr(device, args->data, sizeof(target))))
        return error;

    KASSERT(t->thr_proc->p_rootdir != NULL);

    if ((error = vfs_lookup(t->thr_proc->p_curdir, &target_node, target, t,
      LKP_NORMAL))) {
        return error;
    }

    if ((error = vnode_opendev(device, 0, &device_node))) {
        goto out;
    }

    vfs_mount(type, target_node, device_node->v_dev);

out:
    vrele(target_node);
    return error;
}
