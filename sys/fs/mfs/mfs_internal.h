#ifndef __FS_MFS_INTERNAL_H
#define __FS_MFS_INTERNAL_H
#ifdef __KERNEL

typedef struct mfs_node mfs_node_t;
typedef struct mfs_blk mfs_blk_t;

struct mfs_node {
    char            *name;
    size_t          size;
    size_t          alloc_size;
    int             type;
    int             attr;
    uid_t           uid;
    gid_t           gid;
    unsigned char   *data;
    mfs_data_t      *mfs;
    mfs_node_t      *parent;
    mfs_node_t      *child;
    mfs_node_t      *next;
    vnode_t         *vnode;
    list_t          blks;
};

struct mfs_blk {
    list_node_t     L_blks;
    char            data[MFS_BLOCK_SIZE];
};

struct mfs_data {
    vnode_t         *rootvnode;
    mfs_node_t      *rootinode;
    kmem_cache_t    *cache_blk;
};


int mfs_blk_write(mfs_node_t *n, uio_t *uio);
int mfs_blk_read(mfs_node_t *n, uio_t *uio);
int mfs_blk_set_area(mfs_node_t *n, size_t s);
int mfs_nodecreate(vnode_t *, vnode_t **, const char *, vattr_t *);
mfs_node_t* _alloc_node(void);
int _get_vnode(mfs_node_t *node, vnode_t **vpp, vfs_t *fs);

extern vnode_ops_t mfs_vnode_ops;
#endif
#endif

