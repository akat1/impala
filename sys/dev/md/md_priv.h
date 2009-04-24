/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#ifndef __DEV_MD_PRIV_H
#define __DEV_MD_PRIV_H

enum MD_DATA_TYPE {
    MD_DATA_TYPE_ALLOCATED,
    MD_DATA_TYPE_FOREIGN 
};

typedef struct memdisk memdisk_t;
struct memdisk {
    int             unit;
    size_t          size;
    void           *data;
    int             data_type;
    const char     *descr;
    devd_t         *devd; 
    thread_t       *owner;
    list_node_t     L_memdisks;
};



#endif
