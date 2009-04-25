/* Impala Operating System
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://trzask.codepainters.com/impala/trac/
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

