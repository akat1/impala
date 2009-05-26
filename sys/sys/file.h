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

#ifndef __SYS_FILE_H
#define __SYS_FILE_H

#include <sys/list.h>
#include <sys/types.h>

enum {
    FILES_PER_CHUNK = 32
};

struct filetable {
    int     max_ds;
    list_t  chunks;
};

struct filetable_chunk {
    file_t      *files[FILES_PER_CHUNK];
    list_node_t L_chunks;
};

struct file {
    vnode_t    *f_vnode;
    off_t       f_offset;
    int         f_refcnt;
    mode_t      f_openmode;
    int         f_flags;
    //pcred_t   *f_pcred;
};



#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2


#define O_RDONLY      (1 << 1)
#define O_WRONLY      (1 << 2)
#define O_RDWR        (1 << 3)
#define O_CREAT       (1 << 4)

#define F_DUPFD       (1 << 0)
#define F_GETFL       (1 << 1)
#define F_SETFL       (1 << 2)


#ifdef __KERNEL
/* XXX */
enum {
    OK,
    MAX_EXCEEDED
};

void filetable_free(filetable_t *fd);
filetable_t *filetable_alloc(void);

int     f_alloc(proc_t *p, vnode_t  *vn, file_t **fpp, int *result);
ssize_t f_write(file_t *fd, uio_t *u);
ssize_t f_read(file_t *fd, uio_t *u);
int     f_ioctl(file_t *fd, int cmd, uintptr_t param);
int     f_fcntl(filetable_t *ft, file_t *fp, int cmd, uintptr_t param);
void    f_close(file_t *fd);
off_t   f_seek(file_t *fd, off_t o, int whence);

// nie ma sensu ich dostępniać moim zdaniem.
file_t *f_get(filetable_t *ft, int index);
void f_set(filetable_t *ft, file_t *fd, int index);

#endif

#endif
