#ifndef __FS_MFS_INTERNAL_H
#define __FS_MFS_INTERNAL_H

typedef struct mfs_header mfs_header_t;
typedef struct mfs_data_entry mfs_data_entry_t;
typedef struct mfs_node mfs_node_t;
typedef struct mfs_data mfs_data_t;

enum {
    MFS_MAGIC0          = 0x43214321,
    MFS_MAGIC1          = 0x76576576
};

enum {
    MFS_MAX_PATH        = 256,
    MFS_MAX_FNAME       = 64
};

enum {
    MFS_TYPE_REG    = 0,
    MFS_TYPE_DIR    = 1,
    MFS_TYPE_LNK    = 2,
    MFS_TYPE_XXX    = 3
};

enum {
    MFS_ATTR_OWNER_R  = 0400,
    MFS_ATTR_OWNER_W  = 0200,
    MFS_ATTR_OWNER_X  = 0100,
    MFS_ATTR_GROUP_R  =  040,
    MFS_ATTR_GROUP_W  =  020,
    MFS_ATTR_GROUP_X  =  010,
    MFS_ATTR_OTHER_R  =   04,
    MFS_ATTR_OTHER_W  =   02,
    MFS_ATTR_OTHER_X  =   01
};

struct mfs_header {
    uint32_t    magic0;
    uint32_t    magic1;
    uint16_t    items;
};

struct mfs_data_entry {
    char     name[MFS_MAX_FNAME];
    size_t   size;
    uint32_t type;
    uint32_t attr;
    uint32_t data_off;
    uint32_t parent_id;
    uint32_t child_id;
    uint32_t next_id;
};

struct mfs_node {
    char       *name;
    size_t      size;
    int         type;
    int         attr;
    unsigned char *data;
    mfs_node_t *parent;
    mfs_node_t *child;
    mfs_node_t *next;
    vnode_t    *vnode;
};



#endif

