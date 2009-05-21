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

