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
#include <sys/vfs.h>
#include <sys/file.h>
#include <sys/proc.h>
#include <sys/errno.h>

bool _chunk_is_empty(filetable_chunk_t *fd);
static void _filetable_expand(filetable_t *ft, int hm);
static filetable_chunk_t *_get_chunk_by_index(filetable_t *ft, int index);
static file_t *file_alloc(vnode_t *vn, int flags);
static void f_set_cl(filetable_t *ft, int index, bool cloexec);
static bool f_get_cl(filetable_t *ft, int index);

file_t*
file_alloc(vnode_t *vn, int flags)
{
    KASSERT(vn);
    file_t *fp = kmem_zalloc(sizeof(file_t), KM_SLEEP);
    if(!fp)
        return NULL;
    fp->f_vnode = vn;
    fp->f_flags = flags;
    fp->f_refcnt++;
    return fp;
}

void 
fref(file_t *f)
{
    KASSERT(f->f_refcnt>0);
    f->f_refcnt++;
}

bool
frele(file_t *f)
{
    KASSERT(f->f_refcnt>0);
    f->f_refcnt--;

    if ( f->f_refcnt == 0 ) {
        vrele(f->f_vnode);
        kmem_free(f);
        return TRUE;
    }

    return FALSE;
}

off_t
f_seek(file_t *f, off_t o, int whence)
{
    vattr_t attr;

    if ( f == NULL ) {
        return -EBADF;
    }
    off_t oldoff = f->f_offset;
    switch(whence) {
        case SEEK_SET:
            f->f_offset = o;
            break;
        case SEEK_CUR:
            f->f_offset += o;
            break;
        case SEEK_END:
            attr.va_mask = VATTR_SIZE;
            VOP_GETATTR(f->f_vnode, &attr);
            if ( attr.va_size - o < 0 )
                f->f_offset = 0;
            else
                f->f_offset = attr.va_size - o;
            break;
        default:
            return -EINVAL;
    }
    if(!VOP_SEEK(f->f_vnode, f->f_offset)) //is this offset OK?
        return f->f_offset; //Yes, it is
    f->f_offset = oldoff;
    return f->f_offset;
}

int
f_ioctl(file_t *f, int cmd, uintptr_t param)
{
    if ( f == NULL )
        return -EBADF;
    KASSERT(f->f_vnode);

    return VOP_IOCTL(f->f_vnode, cmd, param);
}

int
f_truncate(file_t *f, off_t len)
{
    if (!f || !(f->f_flags & O_WRONLY || f->f_flags & O_RDWR))
        return -EBADF;
    KASSERT(f->f_vnode);
    return VOP_TRUNCATE(f->f_vnode, len);
}

ssize_t
f_write(file_t *f, uio_t *u)
{
    if(!(f->f_flags & O_WRONLY || f->f_flags & O_RDWR))
        return -EBADF;
    u->offset = f->f_offset;
    int res = VOP_WRITE(f->f_vnode, u, f->f_flags);
    if(res < 0) 
        return res;
    f->f_offset += res;
    return res;
}

ssize_t
f_read(file_t *f, uio_t *u)
{
    if(!(f->f_flags & O_RDONLY || f->f_flags & O_RDWR))
        return -EBADF;
    u->offset = f->f_offset;
    int res = VOP_READ(f->f_vnode, u, f->f_flags);
    if(res < 0)
        return res;
    f->f_offset += res;
    return res;
}

int
f_fcntl(filetable_t *ft, int argfd, file_t *f, int cmd, uintptr_t param)
{
    int fd;
    filetable_chunk_t *fc;
    switch(cmd) {
        case F_DUPFD:
            if ( param > ft->max_ds )    
                return -EINVAL;
            fc = _get_chunk_by_index(ft, (int)param);
            if ( fc == NULL ) {
                _filetable_expand(ft, param - list_length(&(ft->chunks))
                                              / FILES_PER_CHUNK);
                fc = _get_chunk_by_index(ft, param);
            }
            fd = param;
            while ( fc != NULL ) {
                for (int i = param % FILES_PER_CHUNK; i < FILES_PER_CHUNK; i++){
                    MUTEX_LOCK(&ft->mtx, "f_fcntl");
                    if ( fc->files[i] == NULL ) {
                        fc->files[i] = f;
                        fc->close_flag[i] = FALSE;
                        fref(f);
                        mutex_unlock(&ft->mtx);
                        return fd;
                    }
                    mutex_unlock(&ft->mtx);
                    fd++;
                }
                if ( fd > ft->max_ds )
                    return -EMFILE;
                fc = (filetable_chunk_t *)list_next(&(ft->chunks), fc);
            }
            //shm: todo: a co je¶li nie by³o miejsca?
            panic("FCNTL not impl opt?\n");
            break;
        case F_GETFL:
            return f->f_flags;
        case F_SETFL: {
            int possible = O_APPEND | O_NONBLOCK;
            f->f_flags = (f->f_flags & ~possible) | (param & possible);
            return 0;
        }
        case F_SETFD:
            f_set_cl(ft, argfd, param);
            return 0;
        case F_GETFD:
            return f_get_cl(ft, argfd);
            break;
    }
    return -1;
}


file_t *
f_get(filetable_t *ft, int index)
{
    if(index<0)
        return NULL;
    MUTEX_LOCK(&ft->mtx, "f_get");
    filetable_chunk_t *fc = _get_chunk_by_index(ft, index);

    if ( fc == NULL ) {
        mutex_unlock(&ft->mtx);
        return NULL;
    }
    file_t *res = fc->files[index % FILES_PER_CHUNK];
    // zgodnie z filozofi±, zwracanie zliczanego obiektu z funkcji daje go
    // ze zwiêkszonym licznikiem referencji
    // w ten sposób ograniczamy mo¿liwe wy¶cigi do miejsc takich jak to ;)
    if(res)
        fref(res);
    mutex_unlock(&ft->mtx);
    return res;
}

/**
 * f_set przejmuje prawo w³asno¶ci do fd od wywo³uj±cego i umieszcza wskazywany
 * plik w tablicy otwartych plików procesu
 *
 */

void 
f_set(filetable_t *ft, file_t *fd, int index, bool cloexec)
{
    KASSERT(index>=0);
    MUTEX_LOCK(&ft->mtx, "f_set");
    filetable_chunk_t *fc = _get_chunk_by_index(ft, index);
//    KASSERT(fd == NULL || fd->f_vnode);

    if ( fc == NULL ) {
        _filetable_expand(ft, 1 + index/FILES_PER_CHUNK 
                                - list_length(&(ft->chunks)));
        fc = _get_chunk_by_index(ft, index);
    }
//    KASSERT(fc!=NULL);
    // f_set przejmuje prawo w³asno¶ci do fd od wywo³uj±cego ->
    // wywo³uj±cy przekaza³ nam prawo w³asno¶ci do tej referencji
    file_t *old = fc->files[index % FILES_PER_CHUNK];
    fc->files[index % FILES_PER_CHUNK] = fd;
    fc->close_flag[index % FILES_PER_CHUNK] = cloexec;
    mutex_unlock(&ft->mtx);
    if(old)
        frele(old); //pozbywamy siê naszej w³asno¶ci
}

void
f_set_cl(filetable_t *ft, int index, bool cloexec)
{
    KASSERT(index>=0);
    MUTEX_LOCK(&ft->mtx, "f_set_cl");
    filetable_chunk_t *fc = _get_chunk_by_index(ft, index);
    KASSERT(fc!=NULL);
    fc->close_flag[index % FILES_PER_CHUNK] = cloexec;
    mutex_unlock(&ft->mtx);
}

bool
f_get_cl(filetable_t *ft, int index)
{
    KASSERT(index>=0);
    filetable_chunk_t *fc = _get_chunk_by_index(ft, index);
    KASSERT(fc!=NULL);
    return fc->close_flag[index % FILES_PER_CHUNK];
}

filetable_chunk_t *
_get_chunk_by_index(filetable_t *ft, int index)
{
    filetable_chunk_t *fc = (filetable_chunk_t *)list_head(&(ft->chunks));
    int current = 0;

    if ( fc == NULL )
        return NULL;

    while ( fc != NULL )
    {
        current += FILES_PER_CHUNK;
        if ( index < current )
            return fc;
        fc = (filetable_chunk_t *)list_next(&(ft->chunks), fc);
    }

    /* NOT REACHED */
    return NULL;
}

bool
_chunk_is_empty(filetable_chunk_t *chunk)
{
    for (int i = 0 ; i < FILES_PER_CHUNK ; i++ )
        if ( chunk->files[i] != NULL )
            return FALSE;

    return TRUE;
}

/* rozszerza rozmiar listy z chunkami o hm chunkow */

void
_filetable_expand(filetable_t *ft, int hm)
{
    filetable_chunk_t *fc;

    while (hm--) {
        fc = kmem_zalloc(sizeof(filetable_chunk_t), KM_SLEEP);
        list_insert_tail(&(ft->chunks), fc);
    }

    return;
}

int
f_alloc(proc_t *p, vnode_t  *vn, int flags, int *result)
{
    int fdp = 0;
    file_t *fp;
    filetable_chunk_t *fc;
    KASSERT(vn);
    MUTEX_LOCK(&p->p_fd->mtx, "f_alloc");
    /* sprawdzamy czy to pierwszy deskyptor */
    if ( list_is_empty(&(p->p_fd->chunks)) ) {
        _filetable_expand(p->p_fd, 1);
    }

    /* szukamy miejsca dla nowego deskryptora w chunkach */
    fc = list_head(&(p->p_fd->chunks));

    {
        for (int i = 0 ; i < FILES_PER_CHUNK ; i++) {
            if ( fc->files[i] == NULL ) {
                fp = file_alloc(vn, flags);
                fc->files[i] = fp;
                fc->close_flag[i] = (flags & O_CLOEXEC)>0;
                *result = fdp;

                mutex_unlock(&p->p_fd->mtx);
                return 0;
            }
            fdp++;
        }

        if ( fdp >= p->p_fd->max_ds ) {
            mutex_unlock(&p->p_fd->mtx);
            return -EMFILE;
        }

    } while ( (fc = (filetable_chunk_t *)list_next(&(p->p_fd->chunks), fc)) )

    /* brak wolnych miejsc w chunkach */
    _filetable_expand(p->p_fd, 1);
    fp = file_alloc(vn,flags);
    *result = fdp;
    f_set(p->p_fd, fp, fdp, (flags & O_CLOEXEC)>0);
    mutex_unlock(&p->p_fd->mtx);
    return 0;
}

void
f_close(file_t *fp)
{
    KASSERT(fp);
    KASSERT(fp->f_vnode);
    VOP_CLOSE(fp->f_vnode);
    frele(fp);
    return;
}

filetable_t *
filetable_alloc(void)
{
    filetable_t *t = kmem_zalloc(sizeof(filetable_t), KM_SLEEP);
    t->max_ds = 100;
    LIST_CREATE(&(t->chunks), filetable_chunk_t, L_chunks, FALSE);
    mutex_init(&t->mtx, MUTEX_NORMAL);
    return t;
}

void
filetable_clone(filetable_t *dst, filetable_t *src)
{
    MUTEX_LOCK(&src->mtx, "filetable_clone");
    MUTEX_LOCK(&dst->mtx, "filetable_clone");
    filetable_chunk_t *t = NULL;

    while((t = (filetable_chunk_t*)list_next(&(src->chunks), t))) {
        filetable_chunk_t *nc= kmem_zalloc(sizeof(filetable_chunk_t), KM_SLEEP);
        for ( int i = 0 ; i < FILES_PER_CHUNK ; i++ ) {
            if ( t->files[i] != NULL ) {
                fref(t->files[i]);
//                KASSERT(t->files[i]->f_vnode);
            }
            nc->files[i] = t->files[i];
            nc->close_flag[i] = t->close_flag[i];
        }
        list_insert_tail(&(dst->chunks), nc);
    }
    mutex_unlock(&dst->mtx);
    mutex_unlock(&src->mtx);
}


void
filetable_close(filetable_t *fd)
{
    filetable_chunk_t *t;
    MUTEX_LOCK(&fd->mtx, "filetable_close");
    while((t = (filetable_chunk_t *)list_extract_first(&(fd->chunks)))) {
        for ( int i = 0 ; i < FILES_PER_CHUNK ; i++ ) {
            if ( t->files[i] != NULL ) {
//                kprintf("=FTC - closing: %p\n", t->files[i]);
                f_close(t->files[i]);
            }
        }
        kmem_free(t);
    }
    mutex_unlock(&fd->mtx);
//    KASSERT(list_length(&(fd->chunks))==0);
}

void filetable_prepare_exec(filetable_t *fd)
{
    filetable_chunk_t *t = NULL;
    MUTEX_LOCK(&fd->mtx, "filetable_prepare_exec");
    while((t = (filetable_chunk_t *)list_next(&(fd->chunks), t))) {
        for ( int i = 0 ; i < FILES_PER_CHUNK ; i++ ) {
            if(t->files[i] != NULL && t->close_flag[i]) {
                f_close(t->files[i]);
                t->files[i] = NULL;
            }
        }
    }
    mutex_unlock(&fd->mtx);
}

void
filetable_free(filetable_t *fd)
{
    filetable_close(fd);
    kmem_free(fd);

    return;
}
