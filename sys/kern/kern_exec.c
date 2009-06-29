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

enum {
    MAX_ARGV    = 256,
    MAX_ENVP    = 256
};

void __thread_enter(thread_t *t);

typedef struct exec_info exec_info_t;
struct exec_info {
    const char *path;
    uintptr_t  *argv;
    int         argc;
    uintptr_t  *envp;
    int         envc;
    vnode_t *vp;
    void    *header;
    size_t  header_size;
    size_t  image_size;
    bool    interpreted;
    char   *safe_argv[PAGE_SIZE/sizeof(char*)];
    char    argv_data[PAGE_SIZE];
    size_t  argv_size;
    char   *safe_envp[PAGE_SIZE/sizeof(char*)];
    char    envp_data[PAGE_SIZE];
    size_t  envp_size;
    vm_addr_t   u_argv;
    vm_addr_t   u_envp;
    vm_size_t   u_off;
};

static int image_exec(proc_t *, exec_info_t *einfo);
static int aout_exec(proc_t *, exec_info_t *einfo);
static int intrp_exec(proc_t *, exec_info_t *einfo);
static void prepare_process(proc_t *p);

static int copyin_params(proc_t *p, exec_info_t *);
static int copyout_params(thread_t *t, exec_info_t *);

/*========================================================================
 * EXEC
 */

/**
 * Zastêpuje obraz programu w procesie nowym.
 * @param p proces.
 * @param path ¶cie¿ka do nowego obrazu.
 * @param argv tablica parametrów, dane u u¿ytkownika
 * @param envp ¶rodowisko, dane u u¿ytkownika
 *
 * Najpierw wczytujemy 1024 pliku (nag³ówek), sprawdzaj±c przy okazji
 * czy mamy odpowiednie prawa. Nastêpnie kopiujemy argumenty i ¶rodowisko
 * do j±dra. Nastêpnie dopasowywany jest odpowiedni interpreter obrazu
 * (a.out lub skrypt).
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
    mem_zero(&einfo, sizeof(einfo));
    if ( (err = vfs_lookup(p->p_curdir, &vp, path, NULL, LKP_NORMAL)) ) {
        TRACE_IN("cannot lookup");
        return err;
    }
    vattr_t attr;

    if((err = VOP_ACCESS(vp, X_OK, p->p_cred))) {
        kprintf("Execve failed - perm. denied\n");
        return err;
    }
    attr.va_mask = VATTR_SIZE;
    VOP_GETATTR(vp, &attr);
    einfo.image_size = attr.va_size;
    if (attr.va_size < HEADER_SIZE)
        len = attr.va_size;

    einfo.argv = (uintptr_t *)argv;
    einfo.envp = (uintptr_t *)envp;
    if ( (err = copyin_params(p, &einfo)) ) goto fail;
    len = vnode_rdwr(UIO_READ, vp, header, len, 0);
    if (len < 0) {
        TRACE_IN("bad length");
        err = len;
        goto fail;
    } else
    if (len == 0) {
        TRACE_IN("uio error");
        err = -EINVAL;
        goto fail;
    }


    einfo.interpreted = FALSE;
    einfo.path = path;
    einfo.vp = vp;
    einfo.header = header;
    einfo.header_size = len;
    if ( (err = image_exec(p, &einfo)) ) goto fail;
    if (p->p_cmd) kmem_free((void*)p->p_cmd);
    p->p_cmd = str_dup(path);
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
    // sprawdzamy wszystkie znany interpretery obrazów, wszystkie dwa :)
    int err = -EINVAL;
    if (!einfo->interpreted)
        err = intrp_exec(p, einfo);
    if (err == -EINVAL)
        err = aout_exec(p, einfo);
    return err;
}


int
copyin_params(proc_t *p, exec_info_t *einfo)
{
    int argc = 0;
    int envc = 0;
    size_t cur = 0;

    if (einfo->argv) {
        for (int i = 0; i < MAX_ARGV && cur < PAGE_SIZE;    i++, argc++) {
            if( vm_is_avail((vm_addr_t)&(einfo->argv[i]), sizeof(char*)) )
                return -EFAULT;
            char *uaddr = (char*) einfo->argv[i];
            if(uaddr == NULL)
                break;
            einfo->safe_argv[i] = (char*)cur;
            copyinstr(&einfo->argv_data[cur], uaddr, PAGE_SIZE-cur);
            cur += str_len(uaddr) + 1;
        }
        einfo->safe_argv[argc] = 0;
        einfo->argc = argc;
        einfo->argv_size = cur;
    }
    if (!einfo->envp)
        return 0;
    cur = 0;
    for (int i = 0; i < MAX_ARGV && cur < PAGE_SIZE;
            i++, envc++) {
        if( vm_is_avail((vm_addr_t)&(einfo->envp[i]), sizeof(char*)) )
            return -EFAULT;        
        char *uaddr = (char*) einfo->envp[i];
        if(uaddr == NULL)
            break;

        einfo->safe_envp[i] = (char*)cur;
        copyinstr(&einfo->envp_data[cur], uaddr, PAGE_SIZE-cur);
        cur += str_len(uaddr) + 1;
    }
    einfo->safe_envp[envc] = 0;
    einfo->envp_size = cur;
    einfo->envc = envc;
    return 0;
}

int
copyout_params(thread_t *t, exec_info_t *einfo)
{
    einfo->u_argv = einfo->u_envp = 0;
    if (einfo->argv_size + einfo->envp_size == 0) return 0;
    vm_addr_t MAP;
    char *STACK;
    char *_STACK;
    vm_addr_t addr = (vm_addr_t)t->thr_stack + t->thr_stack_size - 3*PAGE_SIZE;
    // odwzorowujemy 3 ostatnie strony stosu (tzn pierwsze, patrz±c od koñca)
    if (vm_segmap(t->vm_space->seg_stack, addr, 3*PAGE_SIZE, &MAP)) {
        panic("vm_segmap failed");
    }
    STACK = (char*)MAP + 3*PAGE_SIZE;
    _STACK = (char*)addr + 3*PAGE_SIZE;
    if (einfo->argv_size > 0) {
        STACK -= einfo->argv_size;
        _STACK -= einfo->argv_size;
        mem_cpy(STACK, einfo->argv_data, einfo->argv_size);
        for (int i = 0; i < einfo->argc; i++) {
            einfo->safe_argv[i] += (uintptr_t)_STACK;
        }
        STACK -= (einfo->argc+1) * sizeof(char*);
        _STACK -= (einfo->argc+1) * sizeof(char*);
        mem_cpy(STACK, einfo->safe_argv, (einfo->argc+1) * sizeof(char*));
        einfo->u_argv = (vm_addr_t)_STACK;
    }
    if (einfo->envp_size > 0) {
        STACK -= einfo->envp_size;
        _STACK -= einfo->envp_size;
        mem_cpy(STACK, einfo->envp_data, einfo->envp_size);
        for (int i = 0; i < einfo->envc; i++) {
            einfo->safe_envp[i] += (uintptr_t)_STACK;
        }
        STACK -= (einfo->envc+1) * sizeof(char*);
        _STACK -= (einfo->envc+1) * sizeof(char*);
        mem_cpy(STACK, einfo->safe_envp, (einfo->envc+1) * sizeof(char*));
        einfo->u_envp = (vm_addr_t)_STACK;
    }
    einfo->u_off = (MAP+3*PAGE_SIZE) - (vm_addr_t)STACK;
    vm_unmap(MAP, 3*PAGE_SIZE);

    return 0;
}


/*========================================================================
 * Obsluga formatu a.out (tylko ZMAGIC)
 */


int
aout_exec(proc_t *p, exec_info_t *einfo)
{
    if (einfo->image_size < PAGE_SIZE) return -EINVAL;
    exec_t *exec = einfo->header;
    if (N_BADMAG(*exec)) {
        return -EINVAL;
    }
    // skoro plik wydaje siê byæ OK, to bezgranicznie mu zaufajmy

    // zniszczmy aktualny obraz procesu.
    prepare_process(p);

    // tworzymy now± przestrzeñ adresow±, wed³ug danych z pliku.
    vm_space_t *vm_space = p->vm_space;
    vm_seg_create(vm_space->seg_text, vm_space, 0, 0, exec->a_text,
        VM_PROT_RWX|VM_PROT_USER, VM_SEG_NORMAL);
    vm_seg_create(vm_space->seg_data, vm_space, exec->a_text, 0,
        VM_SPACE_KERNEL, VM_PROT_RWX|VM_PROT_USER, VM_SEG_NORMAL);
    vm_seg_create(vm_space->seg_stack, vm_space, VM_SPACE_KERNEL,
        0, 0, VM_PROT_RWX|VM_PROT_USER, VM_SEG_EXPDOWN);

    uintptr_t _TEXT, _DATA;
    void *TEXT, *DATA;
    // przydzielamy odpowiedni± pamiêc na kod i dane.
    vm_seg_alloc(vm_space->seg_text, exec->a_text, &_TEXT);
    vm_seg_alloc(vm_space->seg_data, exec->a_data + exec->a_bss, &_DATA);

    // wmapowujemy przestrzeñ adresow± programu w przestrzeñ j±dra
    // aby skopiowaæ tam dane z pliku. (nie mo¿emy do niej kopiowaæ
    // bezpo¶rednio, bo nie dzia³amy w nowo utworzonej przestrzeni
    // adresowej
    vm_segmap(vm_space->seg_text, _TEXT, exec->a_text, &TEXT);
    vm_segmap(vm_space->seg_data, _DATA, exec->a_data + exec->a_bss, &DATA);
    mem_zero(DATA, exec->a_data + exec->a_bss);

    /// @todo nie sprawdzamy b³êdów I/O, tak czy siak, proces wywo³uj±cy
    ///       execve nie mo¿e ju¿ tego b³êdu obs³u¿yæ, bo nie ma jego
    ///       danych w VM :D
    vnode_rdwr(UIO_READ, einfo->vp, TEXT, exec->a_text, N_TXTOFF(*exec));
    vnode_rdwr(UIO_READ, einfo->vp, DATA, exec->a_data, N_DATAOFF(*exec));

    // niszczymy odwzorowania
    vm_unmap((vm_addr_t)TEXT, exec->a_text);
    vm_unmap((vm_addr_t)DATA, exec->a_data + exec->a_bss);
    thread_t *t = proc_create_thread(p, exec->a_entry);
    vm_space_create_stack(vm_space, &t->thr_stack, thread_stack_size);
    t->thr_stack_size = thread_stack_size;
    p->p_brk_addr = vm_space->seg_data->end;
    p->p_brk_addr = vm_space->seg_data->end;
    copyout_params(t, einfo);
    thread_prepare(t, einfo->u_argv, einfo->u_envp, einfo->u_off);
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

