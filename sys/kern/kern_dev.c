/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
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

