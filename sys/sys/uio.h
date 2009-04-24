#ifndef __SYS_UIO_H
#define __SYS_UIO_H

#ifdef __KERNEL

/*
 * Poniewa¿ iovec jest standardowym typem na UNIXach to istnieje
 * prawdobieñstwo, ¿e jaka¶ aplikacja sama sobie zdefiniuje iovec_t.
 * Zatem nie umieszczamy tego typu w sys/types.h
 *      -- wieczyk
 */
typedef struct iovec iovec_t;
enum UIO_SPACE {
    UIO_USERSPACE,
    UIO_KERNELSPACE
};

enum UIO_OPER {
    UIO_READ,
    UIO_WRITE
};

struct uio {
    iovec_t    *iovs;
    size_t      iovcnt;
    int         space;
    int         oper;
    size_t      size;
    off_t       offset;
};

int uio_copy(void *dstbuf, uio_t *uio, size_t len);

#endif

struct iovec {
    void    *iov_base;
    size_t  iov_len;
};

#endif


