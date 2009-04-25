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
#include <sys/device.h>
#include <sys/kmem.h>
#include <sys/errno.h>
#include <sys/utils.h>
#include <sys/string.h>
#include <sys/kprintf.h>
#include <dev/md/md.h>

static list_t devs;

int
dnotsupported(devd_t *d)
{
    return -ENOTSUP;
}


void
dev_init()
{
    list_create(&devs, offsetof(devd_t, L_devs), FALSE);
    md_init();
}

static bool find_this_dev(const devd_t *d, const char *name);

devd_t *
devd_create(devsw_t *dsw, int unit, void *priv, const char *fmt, ...)
{
    devd_t *dev = kmem_alloc( sizeof(devd_t), KM_SLEEP );   
    if (unit == -1) {
        snprintf(dev->name, DEVD_MAXNAME, "%s", dsw->name);
    } else {
        snprintf(dev->name, DEVD_MAXNAME, "%s%u", dsw->name, unit);
    }
    va_list ap;
    VA_START(ap, fmt);
    vsnprintf(dev->descr, DEVD_MAXDESCR, fmt, ap);
    dev->devsw = dsw;
    dev->priv = priv;
    list_insert_tail(&devs, dev);
    kprintf("%s: %s\n", dev->name, dev->descr);
    return dev;
}

devd_t *
devd_find(const char *name)
{
    uintptr_t u = (uintptr_t)name;
    devd_t *d = list_find(&devs, (list_pred_f*)find_this_dev, (void*)u);
    TRACE_IN("d=%p", d);
    return d;
}

int
devd_open(devd_t *d, int flags)
{
    return d->devsw->d_open(d, flags); 
}

int
devd_read(devd_t *d, uio_t *u)
{
    return d->devsw->d_read(d, u);
}

int
devd_write(devd_t *d, uio_t *u)
{
    return d->devsw->d_write(d, u);
}

int
devd_close(devd_t *d)
{
    return d->devsw->d_close(d);
}

bool
find_this_dev(const devd_t *d, const char *name)
{
    return (str_cmp(d->name, name)==0);
}

