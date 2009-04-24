#ifndef __SYS_FS_MFS_H
#define __SYS_FS_MFS_H

typedef struct mfs_entry mfs_entry;
typedef struct mfs_data_entry mfs_data_entry_t;

enum {
    MFS_MAX_PATH        = 256,
    MFS_MAX_FILENAME    = 100
};

enum {
    MFS_TYPE_REG    = 0,
    MFS_TYPE_DIR    = 1,
    MFS_TYPE_LNK    = 2,
    MFS_TYPE_XXX    = 3
};

enum {
    MFS_ATTR_OWNER_R  = 400,
    MFS_ATTR_OWNER_W  = 0200,
    MFS_ATTR_OWNER_X  = 0100,
    MFS_ATTR_GROUP_R  =  040,
    MFS_ATTR_GROUP_W  =  020,
    MFS_ATTR_GROUP_X  =  010,
    MFS_ATTR_OTHER_R  =   04,
    MFS_ATTR_OTHER_W  =   02,
    MFS_ATTR_OTHER_X  =   01
};

struct mfs_data_entry {
    char    name[MFS_MAX_PATH];
    size_t  size;
    int     type;
    int     attr;
};

#ifdef __KERNEL
struct mfs_entry {
    char        name[MFS_MAX_FILENAME];
    size_t      size;
    void       *data_addr;
    int         type;
    list_t      entries;
    list_node_t entry;
};
#endif


#endif
