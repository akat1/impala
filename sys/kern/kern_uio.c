/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/kprintf.h>
#include <sys/string.h>
#include <sys/utils.h>

static int kernel_copy(char *buf, uio_t *uio, size_t len);

int
uio_copy(void *buf, uio_t *uio, size_t len)
{
    TRACE_IN("buf=%p uio=%p len=%u", buf, uio, len);
    KASSERT(len <= uio->size);
    if (uio->space == UIO_USERSPACE)
        panic("user space I/O not supported yet");
    return kernel_copy((char*)buf, uio, len);
}


int
kernel_copy(char *buf, uio_t *uio, size_t len)
{
    for (int i = 0; i < uio->iovcnt; i++) {
        size_t clen = uio->iovs[i].iov_len;
        if (clen < len) {
            len -= clen;
        } else {
            clen = len;
        }
        if (uio->oper == UIO_WRITE) {
            mem_cpy(buf, uio->iovs[i].iov_base, clen);
        } else {
            mem_cpy(uio->iovs[i].iov_base, buf, clen);
        }
        buf += clen;
    }
    return 0;
}

