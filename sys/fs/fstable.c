#include <sys/types.h>
#include <sys/vfs.h>


vfs_init_t  devfs_init;
vfs_init_t  mfs_init;
vfs_init_t  fatfs_init;

vfs_init_t  *fstab[] = {
    devfs_init,
    mfs_init,
    fatfs_init,
    NULL
};
