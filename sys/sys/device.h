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
#ifndef __SYS_DEVICE_H
#define __SYS_DEVICE_H

#ifdef __KERNEL

enum {
    DEVD_MAXNAME  = 16,
    DEVD_MAXDESCR = 128
};

/// inicjacja sterownika pliku urz�dzenia
typedef void d_init_t(void);
/// operacja otwarcia
typedef int d_open_t(devd_t *d, int flags);
/// operacja wej�cia znakowego
typedef int d_read_t(devd_t *d, uio_t *u, int flags);
/// operacja wyj�cia znakowego
typedef int d_write_t(devd_t *d, uio_t *u, int flags);
/// operacja zamkni�cia
typedef int d_close_t(devd_t *d);
/// operacja kontroli
typedef int d_ioctl_t(devd_t *d, int cmd, uintptr_t param);
/// operacja wej�cia-wyj�cia blokowego
typedef int d_strategy_t(devd_t *d, iobuf_t *b);

/// deska rozdzielcza pliku urz�dzenia.
struct devsw {
    d_open_t        *d_open;        ///< obs�uga otwarcia
    d_close_t       *d_close;       ///< obs�uga zamkni�cia
    d_ioctl_t       *d_ioctl;       ///< obs�uga kontroli
    d_read_t        *d_read;        ///< obs�uga wej�cia
    d_write_t       *d_write;       ///< obs�uga wyj�cia
    d_strategy_t    *d_strategy;    ///< obs�uga wej-wyj blokowego
    int              type;
    const char      *name;          ///< og�lna nazwa plik�w urz.
};

int dnotsupported(devd_t *d);

#define noopen  (d_open_t*) dnotsupported
#define noclose (d_close_t*) dnotsupported
#define noioctl (d_ioctl_t*) dnotsupported
#define noread (d_read_t*) dnotsupported
#define nowrite (d_write_t*) dnotsupported
#define nostrategy (d_strategy_t*) dnotsupported

/// deskryptor pliku urz�dzenia
struct devd {
    int          unit;                  ///< numer jednostki
    char         name[DEVD_MAXNAME];    ///< nazwa
    char         descr[DEVD_MAXDESCR];  ///< opis
    void        *priv;                  ///< prywatny uchwyt sterownika
    int          type;                  ///< rodzaj pliku
    devsw_t     *devsw;                 ///< deska rozdzielcza
    list_node_t  L_devs;
};

enum {
    DEV_CDEV,       ///< urz�dzenie znakowe
    DEV_BDEV,       ///< urz�dzenie blokowe
    DEV_TTY         ///< terminal
};


devd_t *devd_create(devsw_t *dsw, int unit, void *pr);
void devd_destroy(devd_t *dev);
void dev_init(void);
void dev_initdevs(void);
devd_t *devd_find(const char *name);

int devd_open(devd_t *d, int flags);
int devd_close(devd_t *d);
int devd_ioctl(devd_t *d, int cmd, uintptr_t param);
int devd_read(devd_t *d, uio_t *u, int flags);
int devd_write(devd_t *d, uio_t *u, int flags);
int devd_strategy(devd_t *d, iobuf_t *bp);
void devd_printf(devd_t *d, const char *fmt, ...);

#endif

#endif

