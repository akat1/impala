/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/device.h>
#include <sys/kprintf.h>
#include <sys/uio.h>
#include <sys/kmem.h>

static ssize_t fdev_read(filed_t *fd, uio_t *u);
static ssize_t fdev_write(filed_t *fd, uio_t *u);

ssize_t
fdev_read(filed_t *fd, uio_t *u)
{
    ssize_t l;
    if (u->offset == -1) u->offset = fd->data.devd.curpos;
    l = devd_read(fd->data.devd.dev, u);
    if (l > 0) {
        fd->data.devd.curpos += l;
    }
    return l;
}

ssize_t
fdev_write(filed_t *fd, uio_t *u)
{
    ssize_t l;
    if (u->offset == -1) u->offset = fd->data.devd.curpos;
    l = devd_write(fd->data.devd.dev, u);
    if (l > 0) {
        fd->data.devd.curpos += l;
    }
    return l;
}


filed_t *
fd_opendev(const char *name, int flags)
{
    TRACE_IN("name=%s flags=%x", name, flags);
    devd_t *dev = devd_find(name);
    if (dev == NULL) return NULL;
    filed_t *fd = kmem_alloc(sizeof(filed_t), KM_SLEEP);
    if (devd_open(dev, flags)!=0) {
        TRACE_IN("cannot open dev for fd=%p(%s)", fd, name);
        // free(fd);
        return NULL;
    }
    fd->data.devd.dev = dev;
    fd->data.devd.curpos = 0;
    fd->fd_read = fdev_read;
    fd->fd_write = fdev_write;
    return fd;
}


ssize_t 
fd_write(filed_t *fd, uio_t *u)
{
    return fd->fd_write(fd, u);
}

ssize_t
fd_read(filed_t *fd, uio_t *u)
{
    return fd->fd_read(fd, u);
}
