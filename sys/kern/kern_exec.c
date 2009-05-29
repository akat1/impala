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
};

static int aout_exec(proc_t *, exec_info_t *einfo);
static int intrp_exec(proc_t *, exec_info_t *einfo);


/*========================================================================
 * EXEC
 */

int aout_fexec(thread_t *thr, void *first_page);

void
fake_execve(thread_t *t, const void *image, size_t size)
{
     aout_fexec(t, (void*)image);
}


int
execve(thread_t *t, const char *path, char *const argv[], char *const envp[])
{
    proc_t *p = t->thr_proc;
    exec_info_t einfo;
    int err;
    enum {
        HEADER_SIZE = 1024
    };
    char header[HEADER_SIZE];
    ssize_t len = HEADER_SIZE;
    vnode_t *vp = NULL;
    mem_zero(header, HEADER_SIZE);
    if ( (err = vfs_lookup(NULL, &vp, path, NULL)) ) {
        return err;
    }
    vattr_t attr;

    ///@TODO sprawdziæ prawa dostêpu
    attr.va_mask = VATTR_SIZE;
    VOP_GETATTR(vp, &attr);
    einfo.image_size = attr.va_size;
    if (attr.va_size < HEADER_SIZE)
        len = attr.va_size;

    len = vnode_rdwr(UIO_READ, vp, header, len, 0);
    if (len < 0) {
        err = len;
        goto fail;
    }
    if (len == 0) {
        err = -EINVAL;
    }

    einfo.path = path;
    einfo.vp = vp;
    einfo.argv = argv;
    einfo.envp = envp;
    einfo.header = header;
    einfo.header_size = len;

    if ((err = intrp_exec(p, &einfo))) {
        if (err == -EINVAL) {
            if ( (err = aout_exec(p, &einfo)) )
                goto fail;
        }
    }

    vrele(vp);
    sched_exit(t);
    return 0;
fail:
    vrele(vp);
    return err;
}

/*========================================================================
 * Obsluga formatu a.out (ZMAGIC)
 */




int
aout_exec(proc_t *p, exec_info_t *einfo)
{

    if (einfo->header_size < PAGE_SIZE) return -EINVAL;
    exec_t *exec = einfo->header;
    if (N_BADMAG(*exec)) return -EINVAL;


    return -EINVAL;
}


int
aout_fexec(thread_t *thr, void *first_page)
{
    const exec_t * ex = first_page;
    vm_space_t *vm_space = thr->vm_space;
    uintptr_t entry;
    if (N_BADMAG(*ex))
        return -1;
    entry = ex->a_entry;
    const char *text = (void*) (char*)first_page + N_TXTOFF(*ex);
    const char *data = (void*) (char*)first_page + N_DATAOFF(*ex);
    const char *strs = (void*) (char*)first_page + N_STROFF(*ex);
    nlist_t *syms = (void*) (char*)first_page + N_SYMOFF(*ex);

    for (int i = 0; i < (ex->a_syms)/sizeof(nlist_t); i++) {
        if (syms[i].n_type & N_TEXT) {
            const char *name = strs + syms[i].n_un.n_strx;
            if (str_cmp(name, "__start") == 0) {
                entry = syms[i].n_value;
                kprintf("@: %p\n", entry);
            }
        }
    }
    kprintf(".text: %p+%p (%p)\n", N_TXTOFF(*ex), ex->a_text, text);
    kprintf(".entry: %p\n", entry);
    kprintf(".data: %p+%p\n", N_DATAOFF(*ex), ex->a_data);
    kprintf(".syms: %p+%p\n", N_SYMOFF(*ex), ex->a_syms);
    thr->thr_entry_point = (void*)entry;
    vm_seg_create(vm_space->seg_text, vm_space, 0, 0, ex->a_text,
        VM_PROT_RWX|VM_PROT_USER, VM_SEG_NORMAL);
    vm_seg_create(vm_space->seg_data, vm_space, ex->a_text, 0,
        VM_SPACE_KERNEL, VM_PROT_RWX|VM_PROT_USER, VM_SEG_NORMAL);
    vm_seg_create(vm_space->seg_stack, vm_space, VM_SPACE_KERNEL,
        0, 0, VM_PROT_RWX|VM_PROT_USER, VM_SEG_EXPDOWN);


    char *TEXT;
    char *DATA;
    char *BSS;
    vm_seg_alloc(vm_space->seg_text, ex->a_text, &TEXT);
    if (ex->a_data) {
        vm_seg_alloc(vm_space->seg_data, ex->a_data+ex->a_bss, &DATA);
        BSS = DATA + ex->a_data;
        mem_zero(BSS, ex->a_bss);
    }
    vm_space_create_stack(vm_space, &thr->thr_stack, THREAD_STACK_SIZE);
    thr->thr_stack_size = THREAD_STACK_SIZE;
    thr->thr_kstack_size = THREAD_KSTACK_SIZE;
    vm_pmap_switch(&vm_space->pmap);
    mem_cpy(TEXT, (void*)text, ex->a_text);
    if (ex->a_data) {
        mem_cpy(DATA, (void*)data, ex->a_data);
    }
    vm_pmap_switch(&vm_kspace.pmap);
    sched_insert(thr);

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

