/*
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
#include <sys/kthread.h>
#include <sys/clock.h>
#include <sys/sched.h>
#include <sys/device.h>
#include <sys/bio.h>
#include <sys/console.h>
#include <sys/proc.h>
#include <sys/vm.h>
#include <sys/utils.h>
#include <sys/kmem.h>
#include <sys/vfs.h>
#include <sys/proc.h>
#include <sys/string.h>
#include <sys/exec.h>
#include <dev/md/md.h>
#include <fs/devfs/devfs.h>
#include <machine/interrupt.h>
#include <machine/cpu.h>
#include <sys/uio.h>

void kmain(void);
static void print_welcome(void);
static void init_kernel(void);
static void prepare_root(void);
static void start_init_process(void);

void
kmain()
{
    SYSTEM_DEBUG = 1;
    print_welcome();
    init_kernel();
    prepare_root();
    start_init_process();
}

void
prepare_root()
{
}

#if 0

static void PROC_init(void);

void
PROC_init()
{
    __asm__(
        "movl $3, %eax;"
        "int $0x80"
    );

    for (   ;;); __asm__(
        "movl $3, %eax;"
        "int $0x80"
    );
}

char buf[THREAD_STACK_SIZE];
char kbuf[THREAD_KSTACK_SIZE];
#endif

void
start_init_process()
{
#if 0
    proc_t *p = proc_create();
    void *entry = (void*) (VM_SPACE_UTEXT + PAGE_OFF(PROC_init));
    thread_t *t = proc_create_thread(p, THREAD_STACK_SIZE, entry);
    vm_pmap_map(&t->vm_space->pmap, VM_SPACE_UTEXT, &vm_kspace.pmap,
        PTE_ADDR(PROC_init), 2*PAGE_SIZE);
    DEBUGF("big fake: %p -> %p", PROC_init, entry);
    sched_insert(t);
#endif
    vnode_t *fn;
    vfs_lookup(NULL, &fn, "/sbin/init", NULL);
    if(fn) {
        vattr_t attr;
        attr.va_mask = VATTR_SIZE;
        VOP_GETATTR(fn, &attr);
        int isize = attr.va_size;
        unsigned char *img = kmem_alloc(isize, KM_SLEEP);
        vnode_rdwr(UIO_READ, fn, img, isize, 0);
        //kprintf("size: %u, mem: %02x%02x%02x\n", isize, *img, *(img+1), *(img+2));
        thread_t *t = proc_create_thread(initproc, 0);
        fake_execve(t, img, isize);
        panic("Cannot start init process");
    }    
    panic("Cannot find init image");
}

void
print_welcome()
{
    extern int kernel_start, kernel_end;
    kprintf("Copyright (C) 2009\n");
    kprintf("   Mateusz Kocielski, Artur Koninski, Pawel Wieczorek\n");
    kprintf("   http://trzask.codepainters.com/impala/trac/\n");
    kprintf("Copyright (C) 2009\n");
    kprintf("   Department of Computer Science. "
            "University of Wroclaw. Poland.\n");
    kprintf("   http://www.ii.uni.wroc.pl/\n");
    kprintf("----\n");
    kprintf("Impala Operating System, build at %s %s\n", __TIME__, __DATE__);
    kprintf("CPU: %s\n", cpu_i.vendor_string);
    kprintf("kernel loaded at: %p+%u kB\n", &kernel_start,
        ((uintptr_t)&kernel_end - (uintptr_t)&kernel_start)/1024);


}

void
init_kernel()
{
    vm_init();
    kmem_init();
    thread_init();
    sched_init();
    clock_init();
    devfs_init();
    dev_init();
    bio_init();
    vfs_init();
    proc_init();
    cons_init();
    ssleep(1);
    kprintf("kernel initialized\n");
}
