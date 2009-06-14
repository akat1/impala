#ifndef __FS_DEVFS_INTERNAL_H
#define __FS_FIFOFS_INTERNAL_H

#include <sys/clist.h>

typedef struct fifofs_node fifofs_node_t;

enum {
    _MAX_NAME = 64,
    FIFO_SIZE = 16384
};

struct fifofs_node {
/*    char           i_name[_MAX_NAME];
    int            i_attr;
    uid_t          i_uid;
    gid_t          i_gid; */
    vnode_t       *i_readnode;
    vnode_t       *i_writenode;
    clist_t       *i_buf;
};


#endif
