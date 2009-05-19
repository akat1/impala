#include <sys/types.h>
#include <sys/thread.h>
#include <sys/utils.h>
#include <sys/bio.h>
#include <sys/vfs.h>
#include <fs/mfs/mfs.h>
#include <sys/kmem.h>
#include <sys/errno.h>

static vfs_mount_t   mfs_mount;
static vfs_unmount_t mfs_unmount;
static vfs_sync_t    mfs_sync;
static vfs_getroot_t mfs_getroot;

static vfs_ops_t mfs_ops = {
    .vfs_mount = mfs_mount,
    .vfs_unmount = mfs_unmount,
    .vfs_getroot = mfs_getroot,
    .vfs_sync = mfs_sync    
};

static vnode_open_t mfs_v_open;
static vnode_create_t mfs_v_create;
static vnode_close_t mfs_v_close;
static vnode_read_t mfs_v_read;
static vnode_write_t mfs_v_write;
static vnode_ioctl_t mfs_v_ioctl;
static vnode_seek_t mfs_v_seek;
static vnode_truncate_t mfs_v_truncate;
static vnode_getattr_t mfs_v_getattr;
static vnode_setattr_t mfs_v_setattr;

static vnode_ops_t mfs_vnode_ops = {
    .vop_open = mfs_v_open,
    .vop_create = mfs_v_create,
    .vop_close = mfs_v_close,
    .vop_read = mfs_v_read,
    .vop_write = mfs_v_write,
    .vop_ioctl = mfs_v_ioctl,
    .vop_seek = mfs_v_seek,
    .vop_truncate = mfs_v_truncate,
    .vop_getattr = mfs_v_getattr,
    .vop_setattr = mfs_v_setattr,
};

int mfs_v_open(vnode_t *vn, int flags, mode_t mode)
{
    return -ENOTSUP;
}

int mfs_v_create(vnode_t *vn, char *name)
{
    return -ENOTSUP;
}

int mfs_v_close(vnode_t *vn)
{
    return -ENOTSUP;
}

int mfs_v_read(vnode_t *vn, uio_t *u)
{
    return -ENOTSUP;
}

int mfs_v_write(vnode_t *vn, uio_t *u)
{
    return -ENOTSUP;
}

int mfs_v_ioctl(vnode_t *vn, int cmd, uintptr_t arg)
{
    return -ENOTSUP;
}

int mfs_v_seek(vnode_t *vn, off_t off)
{
    return -ENOTSUP;
}

int mfs_v_truncate(vnode_t *vn, off_t off)
{
    return -ENOTSUP;
}

int mfs_v_getattr(vnode_t *vn, vattr_t *attr)
{
    return -ENOTSUP;
}

int mfs_v_setattr(vnode_t *vn, vattr_t *attr)
{
    return -ENOTSUP;
}



struct mfs_data {
    vnode_t *rootnode;
    char    *rawdata;
};

typedef struct mfs_data mfs_data_t;

void
fs_mfs_init()
{
    vfs_register("mfs", &mfs_ops);
}


int
mfs_mount(vfs_t *fs)
{
/*    iobuf_t *bp;
    bp = bio_read(fs->vfs_mdev, 0, 1);
    if (!bp) {
        DEBUGF("cannot start I/O operation");
        return -1;
    }
    char *p = bp->addr;
    DEBUGF("readed %s", p);*/
    mfs_data_t *mfs = kmem_zalloc(sizeof(mfs_data_t), KM_SLEEP);
    mfs->rootnode = NULL;
    fs->vfs_private = mfs;
    return 0;
}

int
mfs_unmount(vfs_t *fs)
{
    return 0;
}

void
mfs_sync(vfs_t *fs)
{
}

vnode_t *
mfs_getroot(vfs_t *fs)
{
    mfs_data_t *mfs = fs->vfs_private;
    if(mfs->rootnode == NULL) {
        vnode_t *rn = vnode_alloc();
        rn->v_ops = &mfs_vnode_ops;
        rn->v_type = VNODE_TYPE_DIR;
    }
    return mfs->rootnode;
}


