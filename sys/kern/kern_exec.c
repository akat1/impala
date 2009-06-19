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
#include <sys/exec.h>
#include <sys/aout.h>
#include <sys/utils.h>
#include <sys/proc.h>
#include <sys/string.h>
#include <sys/vm.h>
#include <sys/errno.h>
#include <sys/kmem.h>
#include <sys/uio.h>
#include <sys/vfs.h>
#include <machine/cpu.h>
#include <machine/thread.h>

typedef struct progam program_t;

void __thread_enter(thread_t *t);

typedef struct exec_info exec_info_t;
struct exec_info {
    const char *path;
    char *const *argv;
    char *const *envp;
    vnode_t *vp;
    void    *header;
    size_t  header_size;
    size_t  image_size;
    bool    interpreted;
};

static int image_exec(proc_t *, exec_info_t *einfo);
static int aout_exec(proc_t *, exec_info_t *einfo);
static int intrp_exec(proc_t *, exec_info_t *einfo);
static void prepare_process(proc_t *p);


/*========================================================================
 * EXEC
 */

int
execve(proc_t *p, const char *path, char *argv[], char *envp[])
{
    exec_info_t einfo;
    int err;
    enum {
        HEADER_SIZE = 1024
    };
    char header[HEADER_SIZE];
    ssize_t len = HEADER_SIZE;
    vnode_t *vp = NULL;
    mem_zero(header, HEADER_SIZE);
    if ( (err = vfs_lookup(p->p_curdir, &vp, path, NULL, LKP_NORMAL)) ) {
        TRACE_IN("cannot lookup");
        return err;
    }
    vattr_t attr;

    ///@todo sprawdziæ prawa dostêpu
    attr.va_mask = VATTR_SIZE;
    VOP_GETATTR(vp, &attr);
    einfo.image_size = attr.va_size;
    if (attr.va_size < HEADER_SIZE)
        len = attr.va_size;

    len = vnode_rdwr(UIO_READ, vp, header, len, 0);
    if (len < 0) {
        TRACE_IN("bad length");
        err = len;
        goto fail;
    }
    if (len == 0) {
        TRACE_IN("uio error");
        err = -EINVAL;
        goto fail;
    }


    einfo.interpreted = FALSE;
    einfo.path = path;
    einfo.vp = vp;
    einfo.argv = argv;
    einfo.envp = envp;
    einfo.header = header;
    einfo.header_size = len;
    if ( (err = image_exec(p, &einfo)) ) goto fail;
    vrele(vp);
    return 0;
fail:
    TRACE_IN("failed");
    vrele(vp);
    return err;
}

void
prepare_process(proc_t *p)
{
    proc_destroy_threads(p);
    proc_reset_vmspace(p);
    filetable_prepare_exec(p->p_fd);
    p->p_flags |= PROC_AFTER_EXEC;
}

int
image_exec(proc_t *p, exec_info_t *einfo)
{
    int err = -EINVAL;
    if (!einfo->interpreted)
        err = intrp_exec(p, einfo);
    if (err == -EINVAL)
        err = aout_exec(p, einfo);
    return err;
}


/*========================================================================
 * Obsluga formatu a.out (ZMAGIC)
 */


int
aout_exec(proc_t *p, exec_info_t *einfo)
{
//     TRACE_IN("p=%p einfo=%p", p, einfo);
    if (einfo->image_size < PAGE_SIZE) return -EINVAL;
    exec_t *exec = einfo->header;
    if (N_BADMAG(*exec)) return -EINVAL;
//     uintptr_t entry = exec->a_entry;

    prepare_process(p);


    vm_space_t *vm_space = p->vm_space;
    vm_seg_create(vm_space->seg_text, vm_space, 0, 0, exec->a_text,
        VM_PROT_RWX|VM_PROT_USER, VM_SEG_NORMAL);
    vm_seg_create(vm_space->seg_data, vm_space, exec->a_text, 0,
        VM_SPACE_KERNEL, VM_PROT_RWX|VM_PROT_USER, VM_SEG_NORMAL);
    vm_seg_create(vm_space->seg_stack, vm_space, VM_SPACE_KERNEL,
        0, 0, VM_PROT_RWX|VM_PROT_USER, VM_SEG_EXPDOWN);


    uintptr_t _TEXT, _DATA;
    void *TEXT, *DATA;

    vm_seg_alloc(vm_space->seg_text, exec->a_text, &_TEXT);
    vm_seg_alloc(vm_space->seg_data, exec->a_data + exec->a_bss, &_DATA);

    vm_segmap(vm_space->seg_text, _TEXT, exec->a_text, &TEXT);
    vm_segmap(vm_space->seg_data, _DATA, exec->a_data + exec->a_bss, &DATA);
    mem_zero(DATA, exec->a_data + exec->a_bss);
//     DEBUGF("reading .text");
    vnode_rdwr(UIO_READ, einfo->vp, TEXT, exec->a_text, N_TXTOFF(*exec));
//     DEBUGF("reading .data");
    vnode_rdwr(UIO_READ, einfo->vp, DATA, exec->a_data, N_DATAOFF(*exec));
//     DEBUGF("preparing proc");
    thread_t *t = proc_create_thread(p, exec->a_entry);
    vm_space_create_stack(vm_space, &t->thr_stack, thread_stack_size);
    t->thr_stack_size = thread_stack_size;
    thread_prepare(t);
    sched_insert(t);

    return 0;
}

/*========================================================================
 * Obsluga interpretera
 */

int
intrp_exec(proc_t *p, exec_info_t *einfo)
{
    char *bytes = einfo->header;
    if (einfo->header_size < 4) return -EINVAL;
    if (bytes[0] != '#' || bytes[1] != '!') return -EINVAL;

    return 0;
}

