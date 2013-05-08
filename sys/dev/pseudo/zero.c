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

#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/device.h>
#include <sys/uio.h>
#include <fs/devfs/devfs.h>


d_init_t zero_init;
d_open_t zero_open;
d_close_t zero_close;
d_write_t zero_write;
d_read_t zero_read;

static devsw_t zero_devsw = {
    zero_open,
    zero_close,
    noioctl,
    zero_read,
    zero_write,
    nostrategy,
    DEV_CDEV,
};


void
zero_init(void)
{
    devd_t *dev = devd_create(&zero_devsw, "zero", -1, NULL);
    devfs_register(dev, 0, 0, 0666);
}


/*========================================================================
 * Obsługa pliku urządzenia znakowego /dev/zero
 */

int
zero_open(devd_t *d, int flags)
{
    return 0;
}

int
zero_close(devd_t *d)
{
    return 0;
}

int
zero_read(devd_t *d, uio_t *u, int flags)
{
    char buf[256];
    mem_zero(buf, sizeof(buf));
    while (u->resid) {
        int n = MIN(sizeof(buf), u->resid);
        uio_move(buf, n, u);
    }
    return u->size;
}

int
zero_write(devd_t *d, uio_t *u, int flags)
{
    u->resid = 0;
    return u->size;
}

