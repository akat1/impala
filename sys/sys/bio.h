/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#ifndef __SYS_BIO_H
#define __SYS_BIO_H
#ifdef __KERNEL

/*
 * Standardowy UNIXowy model mo¿na powiedzieæ.
 *  
 */

struct biobuf {
    void        *addr;
    size_t       totalsize;
    size_t       size;
    size_t       remaining;
    size_t       used;
    int          flags;
    devd_t      *devd;
    list_node_t  L_bioq;
    list_node_t  L_bufs;
};

enum BUF_FLAGS {
    B_BUSY      = 0x00001,
    B_DONE      = 0x00002,
    B_WRITE     = 0x00004,
    B_READ      = 0x00008,
    B_EINTR     = 0x00010,
    B_ERROR     = 0x00020,
    B_DIRTY     = 0x00040,
    B_CACHE     = 0x00080
};

#endif
#endif

