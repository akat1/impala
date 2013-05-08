/* Impala Operating System
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://bitbucket.org/wieczyk/impala/
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

#include <fs/devfs/devfs.h>
#include <sys/types.h>
//#include <sys/console.h>
#include <sys/termios.h>
#include <sys/ctty.h>
#include <sys/kmem.h>
#include <sys/device.h>
#include <sys/string.h>
#include <sys/signum.h>
#include <sys/thread.h>
#include <sys/errno.h>
#include <sys/utils.h>
#include <sys/ioctl.h>
#include <sys/proc.h>
#include <sys/uio.h>
#include <sys/vm.h>
#include <machine/video.h>
#include <machine/interrupt.h>
#include <machine/pckbd.h>


static d_open_t ctty_open;
static d_close_t ctty_close;
static d_write_t ctty_write;
static d_read_t ctty_read;
static d_close_t ctty_close;
static d_ioctl_t ctty_ioctl;


static devsw_t cttysw = {
    ctty_open,
    ctty_close,
    ctty_ioctl,
    ctty_read,
    ctty_write,
    nostrategy,
    DEV_TTY,
};

int
ctty_create(void)
{
    devd_t *dev = devd_create(&cttysw, "tty", -1, NULL);
    if(!dev)
        return -1;
    devfs_register(dev, 0, 0, 0666);
    return 0;
}


/*========================================================================
 * Plik urządzenia: /dev/tty - przekierowanie do terminala kontrolującego
 */

static devd_t * getctty(void);

devd_t *
getctty(void)
{
    if(!curthread)
        return NULL;
    proc_t *p = curthread->thr_proc;
    if(!p || !p->p_ctty)
        return NULL;
    return p->p_ctty->t_dev;
}

int
ctty_open(devd_t *d, int flags)
{
    if(!getctty())
        return -ENOTTY;
    return 0;
}

int
ctty_read(devd_t *d, uio_t *u, int flags)
{
    devd_t *de = getctty();
    if(!de)
        return -ENOTTY;
    return devd_read(de, u, flags);
}

int
ctty_write(devd_t *d, uio_t *u, int flags)
{
    devd_t *de = getctty();
    if(!de)
        return -ENOTTY;
    return devd_write(de, u, flags);
}

int
ctty_ioctl(devd_t *d, int cmd, uintptr_t param)
{
    devd_t *de = getctty();
    if(!de)
        return -ENOTTY;
    return devd_ioctl(de, cmd, param);
}

int
ctty_close(devd_t *d)
{
    devd_t *de = getctty();
    if(!de)
        return -ENOTTY;
    return devd_close(de);
}

