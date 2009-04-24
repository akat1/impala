/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */
#ifndef __SYS_FCNTL_H
#define __SYS_FCNTL_H

#ifdef __KERNEL
typedef int filed_close_t(filed_t *fd);
typedef ssize_t filed_read_t(filed_t *fd, uio_t *u);
typedef ssize_t filed_write_t(filed_t *fd, uio_t *u);
typedef ssize_t filed_fcntl_t(filed_t *fd, int cmd, uintptr_t arg);
typedef ssize_t filed_ioctl_t(filed_t *fd, int cmd, uintptr_t arg);
typedef int filed_truncate_t(filed_t *fd, off_t len);
typedef off_t filed_lseek_t(filed_t *fd, off_t off, int whence);
typedef int filed_chmod_t(filed_t *fd, mode_t mode);
typedef int filed_chown_t(filed_t *fd, uid_t owner, gid_t group);

struct filed {
    filed_read_t      *fd_read;
    filed_write_t     *fd_write;
    filed_ioctl_t     *fd_ioctl;
    filed_fcntl_t     *fd_fcntl;
    filed_lseek_t     *fd_lseek;
    filed_truncate_t  *fd_truncate;
    filed_chmod_t     *fd_chmod;
    filed_chown_t     *fd_chown;
    filed_close_t     *fd_close;
    union {
        void          *priv;
        struct {
            devd_t     *dev;
            off_t      curpos;
        }              devd;
    } data;
    int                fd;
};

filed_t *fd_opendev(const char *name, int flags);
ssize_t fd_write(filed_t *fd, uio_t *u);
ssize_t fd_read(filed_t *fd, uio_t *u);
int fd_ioctl(filed_t *fd, int cmd, uintptr_t param);
int fd_close(filed_t *fd);

ssize_t fd_write1(filed_t *fd, const void *buf, size_t len);
ssize_t fd_read1(filed_t *fd, void *buf, size_t len);
#endif

#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2


#define O_RDONLY      (1 << 1)
#define O_WRONLY      (1 << 2)
#define O_RDWR        (1 << 3)
#define O_CREAT       (1 << 4)

#endif

