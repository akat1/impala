#include <sys/types.h>
#include <sys/thread.h>
#include <sys/kprintf.h>
#include <sys/bio.h>
#include <sys/vfs.h>
#include <fs/mfs/mfs.h>

static vfs_mount_t  mfs_mount;
static vfs_unmount_t mfs_unmount;

static vfs_ops_t mfs_ops = {
    mfs_mount,
    mfs_unmount
};

void
fs_mfs_init()
{
    vfs_register("mfs", &mfs_ops);
}


int
mfs_mount(vfs_mp_t *mp)
{
    iobuf_t *bp;
    bp = bio_read(mp->dev_node, 0, 1);
    if (!bp) {
        DEBUGF("cannot start I/O operation");
        return -1;
    }
    char *p = bp->addr;
    DEBUGF("readed %s", p);
    return 0;
}

int
mfs_unmount(vfs_mp_t *mp)
{
    return 0;
}

