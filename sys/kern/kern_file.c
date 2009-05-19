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
#include <sys/fcntl.h>
#include <sys/device.h>
#include <sys/utils.h>
#include <sys/uio.h>
#include <sys/kmem.h>
#include <sys/file.h>
#include <sys/proc.h>

static ssize_t fdev_read(filed_t *fd, uio_t *u);
static ssize_t fdev_write(filed_t *fd, uio_t *u);
bool __chunk_is_empty(filetable_chunk_t *fd);

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

bool
__chunk_is_empty(filetable_chunk_t *chunk)
{
    for (int i = 0 ; i < FILES_PER_CHUNK ; i++ )
        if ( chunk->files[i] != NULL )
            return FALSE;

    return TRUE;
}

int
fd_alloc(proc_t *p, vnode_t  *vn, file_t **fpp, int *result)
{
    int fdp = 0;
    file_t *fp;
    filetable_chunk_t *fc;

    /* sprawdzamy czy to pierwszy deskyptor */
    if ( list_is_empty(&(p->p_fd->chunks)) )
    {
        fc = kmem_zalloc(sizeof(filetable_chunk_t), KM_SLEEP);
        list_insert_head(&(p->p_fd->chunks), fc);
        fp = kmem_zalloc(sizeof(file_t), KM_SLEEP);
        fp->vn = vn;
    }

    /* szukamy miejsca dla nowego deskryptora w chunkach */
    fc = list_head(&(p->p_fd->chunks));

    {
        for (int i = 0 ; i < FILES_PER_CHUNK ; i++)
        {
            if ( fc->files[i] == NULL )
            {
                fp = kmem_zalloc(sizeof(file_t), KM_SLEEP);
                fp->vn = vn;
                *result = fdp;
                *fpp = fp;
                return FD_ALLOC_OK;
            }

            fdp++;
        }

        if ( fdp >= p->p_fd->max_ds )
            return FD_ALLOC_MAX_EXCEEDED;

    } while ( (fc = (filetable_chunk_t *)list_next(&(p->p_fd->chunks), fc)) )

    /* brak wolnych miejsc w chunkach */
    fc = kmem_zalloc(sizeof(filetable_chunk_t), KM_SLEEP);
    list_insert_tail(&(p->p_fd->chunks), fc);
    fp = kmem_zalloc(sizeof(file_t), KM_SLEEP);
    fp->vn = vn;
    *result = fdp;
    *fpp = fp;

    return FD_ALLOC_OK;
}

void
fd_close(file_t *fp)
{
    fp->ref_cnt--;
 
    if ( fp->ref_cnt == 0 )
        kmem_free(fp);

    return;
}

filetable_t *
filetable_alloc(void)
{
    filetable_t *t = kmem_alloc(sizeof(filetable_t), KM_SLEEP);

    LIST_CREATE(&(t->chunks), filetable_chunk_t, L_chunks, FALSE);

    return t;
}

void
filetable_free(filetable_t *fd)
{
    filetable_chunk_t *t;

    while((t = (filetable_chunk_t *)list_extract_first(&(fd->chunks))))
    {
        for ( int i = 0 ; i < FILES_PER_CHUNK ; i++ )
        {
            /* sprawdzamy czy jest otwarty plik */
            if ( t->files[i] != NULL )
                fd_close(t->files[i]);
        }

        /* zwalniamy chunka */
        kmem_free(t);
    }

    /* zwalniamy tablicê */
    kmem_free(fd);

    return;
}
