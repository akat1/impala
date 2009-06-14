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

/** @mainpage
 * Dokumentacja systemu operacyjnego Impala
 *
 */

#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/kthread.h>
#include <sys/clock.h>
#include <sys/device.h>
#include <sys/bio.h>
#include <sys/console.h>
#include <sys/vm.h>
#include <sys/ipc.h>
#include <sys/kargs.h>
#include <sys/vfs.h>
#include <sys/exec.h>
#include <dev/md/md.h>
#include <fs/devfs/devfs.h>
#include <machine/interrupt.h>
#include <machine/cpu.h>
#include <machine/pckbd.h>
#include <machine/bus/isa.h>

void kmain(void);
static void print_welcome(void);
static void init_kernel(void);
static void prepare_root(void);
static void start_init_process(void);



void
kmain()
{
    print_welcome();
    init_kernel();
    prepare_root();
    start_init_process();
}

void
prepare_root()
{
    extern unsigned char image[];
    extern unsigned int image_size;
    md_create(0, &image, image_size);
    vfs_mountroot();
    vnode_t *devdir;
    if(!vfs_lookup(NULL, &devdir, "/dev", NULL, LKP_NORMAL))
    {
        vfs_mount("devfs", devdir, NULL);
    } else kprintf("No /dev dir on root filesystem -> devfs not mounted\n");
}

void
start_init_process()
{
    const char *init_path = "/sbin/init";
    initproc = proc_create();
    karg_get_s("init", &init_path);
    kprintf("calling %s\n", init_path);
    int err;
    switch ( (err = execve(initproc, init_path, NULL, NULL)) ) {
        case 0:
            break;
        case -ENOENT:
            panic("Cannot found init image at %s", init_path);
            break;
        default:
            panic("Cannot execute init process");
            break;
    }
    while (1);

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
    SYSTEM_DEBUG = karg_is_set("debug");
    dev_init();
    bio_init();
    vfs_init();
    proc_init();
    cons_init();
    sysvipc_init();
    dev_initdevs();
    SSLEEP(1, "kinit");
    kprintf("kernel initialized\n");
    kprintf("current time: %i\n", curtime.tv_sec);
}
