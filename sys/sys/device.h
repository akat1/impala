/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */
#ifndef __SYS_DEVICE_H
#define __SYS_DEVICE_H

#ifdef __KERNEL

enum {
    DEVD_MAXNAME  = 16,
    DEVD_MAXDESCR = 128
};

typedef int d_open_t(devd_t *d, int flags);
typedef int d_read_t(devd_t *d, uio_t *u);
typedef int d_write_t(devd_t *d, uio_t *u);
typedef int d_close_t(devd_t *d);
typedef int d_ioctl_t(devd_t *d, int cmd, uintptr_t param);
typedef int d_strategy_t(devd_t *d);

struct devsw {
    d_open_t        *d_open;
    d_close_t       *d_close;
    d_ioctl_t       *d_ioctl;
    d_read_t        *d_read;
    d_write_t       *d_write;
    d_strategy_t    *d_strategy;
    const char      *name;
};

int dnotsupported(devd_t *d);

#define noopen  (d_open_t*) dnotsupported
#define noclose (d_close_t*) dnotsupported
#define noioctl (d_ioctl_t*) dnotsupported
#define noread (d_read_t*) dnotsupported
#define nowrite (d_write_t*) dnotsupported
#define nostrategy (d_strategy_t*) dnotsupported

struct devd {
    int          unit;
    char         name[DEVD_MAXNAME];
    char         descr[DEVD_MAXDESCR];
    void        *priv;
    devsw_t     *devsw;
    list_node_t  L_devs;
};

devd_t *devd_create(devsw_t *dsw, int unit, void *pr, const char *fmt, ...);
void devd_destroy(devd_t *dev);
void dev_init(void);
devd_t *devd_find(const char *name);

int devd_open(devd_t *d, int flags);
int devd_close(devd_t *d);
int devd_read(devd_t *d, uio_t *u);
int devd_write(devd_t *d, uio_t *u);

#endif

#endif

