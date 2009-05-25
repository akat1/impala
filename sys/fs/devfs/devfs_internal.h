#ifndef __FS_DEVFS_INTERNAL_H
#define __FS_DEVFS_INTERNAL_H

typedef struct devfs_node devfs_node_t;
typedef struct devfs_data devfs_data_t;
enum {
    _MAX_NAME = 64
};

enum {
    _INODE_TYPE_DEV,
    _INODE_TYPE_DIR
};

struct devfs_node {
    char           i_name[_MAX_NAME];
    int            i_type;
    int            i_attr;
    uid_t          i_uid;
    gid_t          i_gid;
    vnode_t       *i_dirvnode;
    devd_t        *i_dev;
    devfs_node_t  *i_parent;
    devfs_node_t  *i_child;
    devfs_node_t  *i_next;
};

struct devfs_data {
    vnode_t    *rootvnode;
};


#endif
