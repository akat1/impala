/* Impala Operating System
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://bitbucket.org/wieczyk/impala/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id$
 */

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/string.h>
#include <sys/utils.h>

// #define USE_OLD_UIOMOVE

#ifdef USE_OLD_UIOMOVE

static int kernel_copy(char *buf, uio_t *uio, size_t len);

int
uio_move(void *buf, size_t len, uio_t *uio)
{
//    TRACE_IN("buf=%p uio=%p len=%u", buf, uio, len);
    KASSERT(len <= uio->size);
    //if (uio->space == UIO_USERSPACE)
        //panic("user space I/O not supported yet");
    //zakładając, że bufory są sprawdzone, chyba nie ma nic więcej do roboty
    return kernel_copy((char*)buf, uio, len);
}


int
kernel_copy(char *buf, uio_t *uio, size_t len)
{
//    DEBUGF("Kernel memory transfer: (%p+%u) (%p+%u)",
//            buf, len, uio->iovs[0].iov_base, uio->size);
    for (int i = 0; i < uio->iovcnt; i++) {
        size_t clen = uio->iovs[i].iov_len;
        if (clen < len) {
            len -= clen;
        } else {
            clen = len;
        }
        DEBUGF("xfer: %p+%u %u", uio->iovs[i].iov_base, clen,
            uio->oper);
        if (uio->oper == UIO_WRITE) {
            mem_cpy(buf, uio->iovs[i].iov_base, clen);
        } else {
            mem_cpy(uio->iovs[i].iov_base, buf, clen);
        }
        buf += clen;
    }
    return 0;
}

#else

int
uio_move(void *_buf, size_t len, uio_t *uio)
{
    int e;
    char *buf = _buf;
    KASSERT(len <= uio->size);
    while (uio->resid && len) {
        if (uio->iovs->iov_len == 0) {
            uio->iovs++;
            uio->iovcnt--;
        }
        iovec_t *iov = uio->iovs;
//         DEBUGF("xfer: iov(%p+%p)", iov->iov_base, iov->iov_len);
        size_t xfer = MIN(iov->iov_len, len);
//         DEBUGF("xfer: base=%p xfer=%u len=%u resid=%u", iov->iov_base, xfer,len,uio->resid);
        if (uio->space == UIO_SYSSPACE || 1) {
            if (uio->oper == UIO_WRITE) {
                mem_cpy(buf, iov->iov_base, xfer);
            } else {
                mem_cpy(iov->iov_base, buf, xfer);
            }
        } else {
            panic("e");
            if (uio->oper == UIO_READ) {
                e = copyout(iov->iov_base, buf, xfer);
            } else {
                e = copyin(buf, iov->iov_base, xfer);
            }
            if (e < 0) return -1;
        }
        iov->iov_base += xfer;
        iov->iov_len -= xfer;
        uio->resid -= xfer;
        uio->completed += xfer;
        uio->offset += xfer;
        len -= xfer;
    }
    return 0;
}
#endif

