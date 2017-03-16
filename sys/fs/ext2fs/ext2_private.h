/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE"
 * If we meet some day, and you think this stuff is worth it, you can buy us 
 * a beer in return. - AUTHORS
 * ----------------------------------------------------------------------------
 */

#ifndef _SYS_FS_EXT2_PRIVATE_
#define _SYS_FS_EXT2_PRIVATE_
#include <sys/types.h>
#include <fs/ext2fs/ext2.h>

typedef struct ext2fs {
    uint32_t inodes_count;
    uint32_t blocks_count;
    uint32_t blocks_per_group;
    uint32_t inodes_per_group;
    uint32_t frags_per_group;
    uint32_t first_data_block;
	uint32_t block_size;
    uint32_t inode_size;
    uint32_t groups;
    vnode_t *root;
    ext2fs_group_descriptor_t   *group_descriptors;
    vfs_t *vfs;
} ext2fs_t;

typedef struct {
    uint32_t inode_no;
    ext2fs_inode_t *inode;
    ext2fs_t *fs;
    list_t dirnodes;
} ext2fs_node_t;

typedef struct {
    list_node_t L_dirnodes;
    ext2fs_dirnode_t dirnode;
} ext2fs_node_dir_t;

vnode_t *
ext2fs_get_vnode(ext2fs_t *fs, uint32_t inode_no, int v_type);

int ext2fs_read_node(ext2fs_t *fs, uint32_t inode_no, ext2fs_node_t **node);

int ext2fs_read_from_inode(ext2fs_t *fs, ext2fs_inode_t *inode, uint8_t *buf,
  uint32_t block, uint32_t count);

#endif /* ! _SYS_FS_EXT2_PRIVATE_ */
