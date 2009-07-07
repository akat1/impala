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
#ifndef __SYS_UIO_H
#define __SYS_UIO_H

#ifdef __KERNEL

/*
 * Poniewa¿ iovec jest standardowym typem na UNIXach to istnieje
 * prawdobieñstwo, ¿e jaka¶ aplikacja sama sobie zdefiniuje iovec_t.
 * Zatem nie umieszczamy tego typu w sys/types.h
 *      -- wieczyk
 */
typedef struct iovec iovec_t;
enum UIO_SPACE {
    UIO_USERSPACE,
    UIO_SYSSPACE
};

enum UIO_OPER {
    UIO_READ,
    UIO_WRITE
};

/// wej¶cie-wyj¶cie
struct uio {
    iovec_t    *iovs;       ///< tablica buforów
    size_t      iovcnt;     ///< ilo¶c buforów w tablicy
    int         space;      ///< przestrzeñ UIO_SPACE
    int         oper;       ///< operacja UIO_OPER
    size_t      size;       ///< rozmiar
    size_t      resid;      ///< ilo¶æ pozosta³ych danych
    size_t      completed;  ///< ilo¶æ przes³anych danych
    off_t       offset;     ///< przesuniêcie
};

int uio_move(void *dstbuf, size_t len, uio_t *uio);

#endif

struct iovec {
    void    *iov_base;
    size_t  iov_len;
};

ssize_t readv(int, const struct iovec *, int);
ssize_t writev(int, const struct iovec *, int);


#endif


