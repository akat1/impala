/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE"
 * If we meet some day, and you think this stuff is worth it, you can buy us 
 * a beer in return. - AUTHORS
 * ----------------------------------------------------------------------------
 */

#ifndef _SYS_FS_EXT2_
#define _SYS_FS_EXT2_
#include <sys/types.h>

/* DOCS: 
 * http://www.nongnu.org/ext2-doc/ext2.html
 * http://cs.smith.edu/~nhowe/262/oldlabs/ext2.html
 */

/* little endian */
/* XXX: won' work on bigendian */

enum {
    EXT2_MAGIC = 0xEF53
};

enum {
    EXT2_SUPERBLOCK_SIZE = 1024,
    EXT2_SUPERBLOCK_OFFSET = 1024
};

/* super block offsets */
typedef struct __attribute__((packed)) {
	uint32_t sb_inodes_count;
	uint32_t sb_blocks_count;
	uint32_t sb_r_blocks_count;
	uint32_t sb_free_blocks_count;
	uint32_t sb_free_inodes_count;
	uint32_t sb_first_data_block;
	uint32_t sb_log_block_size;
	uint32_t sb_log_frag_size;
	uint32_t sb_blocksb_per_group;
	uint32_t sb_fragsb_per_group;
	uint32_t sb_inodesb_per_group;
	uint32_t sb_mtime;
	uint32_t sb_wtime;
	uint16_t sb_mnt_count;
	uint16_t sb_max_mnt_count;
	uint16_t sb_magic;
	uint16_t sb_state;
	uint16_t sb_errors;
	uint16_t sb_minor_rev_level;
	uint32_t sb_lastcheck;
	uint32_t sb_checkinterval;
	uint32_t sb_creator_os;
	uint32_t sb_rev_level;
	uint16_t sb_def_resuid;
	uint16_t sb_def_resgid;
	uint32_t sb_first_ino;
	uint16_t sb_inode_size;
	uint16_t sb_block_group_nr;
	uint32_t sb_feature_compat;
	uint32_t sb_feature_incompat;
	uint32_t sb_feature_ro_compat;
	uint32_t sb_uuid;
} ext2fs_superblock_t;

typedef struct __attribute__((packed)) {
    uint32_t    block_bitmap;
    uint32_t    inode_bitmap;
    uint32_t    inode_table;
    uint16_t    free_blocks_count;
    uint16_t    free_inodes_count;
    uint16_t    used_dirs_count;
    uint16_t    pad;
    uint32_t    reserved[3];
} ext2fs_group_descriptor_t;

/* blocks in inode will be numbered from 0 */
enum {
    EXT2_BLOCK_MAX_DIRECT       = 11,
    EXT2_BLOCK_INDIRECT         = 12,
    EXT2_BLOCK_DOUBLE_INDIRECT  = 13,
    EXT2_BLOCK_TRIPPLE_INDIRECT = 14
};

typedef struct __attribute__((packed)) {
	uint16_t	mode;
	uint16_t	uid;
	uint32_t	size;
	uint32_t	atime;
	uint32_t	ctime;
	uint32_t	mtime;
	uint32_t	dtime;
	uint16_t	gid; /* gid_t, uid_t etc */
	uint16_t	links_count;
	uint32_t	blocks;
	uint32_t	flags;
	uint32_t	osd1;
	uint32_t	block[15];
	uint32_t	generation;
	uint32_t	file_acl;
	uint32_t	dir_acl;
	uint32_t	faddr;
	uint32_t	osd2[3];
} ext2fs_inode_t; /* XXX: packed */

/* Special inodes */
enum {
	EXT2_INODE_BAD	= 1,
	EXT2_INODE_ROOT = 2
};

/* inode mode values */
enum {
	/* permissions */
	EXT2_S_IXOTH	= 0x0001,
	EXT2_S_IWOTH	= 0x0002,
	EXT2_S_IROTH	= 0x0004,
	EXT2_S_IXGRP	= 0x0008,
	EXT2_S_IWGRP	= 0x0010,
	EXT2_S_IRGRP	= 0x0020,
	EXT2_S_IXUSR	= 0x0040,
	EXT2_S_IWUSR	= 0x0080,
	EXT2_S_IRUSR	= 0x0100,
	
	/* sticky */
	EXT2_S_ISVTX	= 0x0200,
	EXT2_S_ISGID	= 0x0400,
	EXT2_S_ISUID	= 0x0800,

	/* File types */
	EXT2_S_FIFO	    = 0x1000,
	EXT2_S_CHARDEV	= 0x2000,
	EXT2_S_DIR	    = 0x4000,
	EXT2_S_BLKDEV	= 0x6000,
	EXT2_S_REG	    = 0x8000, /* regular file */
	EXT2_S_LINK	    = 0xA000,
	EXT2_S_SOCKET	= 0xC000
};

enum {
    EXT2_MAX_FILELEN = 256
};

typedef struct {
    uint32_t    inode_no;
    uint16_t    entry_size;
    uint8_t     length;
    uint8_t     type;
    char        name[EXT2_MAX_FILELEN];
} ext2fs_dirnode_t;

enum {
    EXT2_D_UNKNOWN    = 0,
    EXT2_D_REG        = 1,
    EXT2_D_DIRECTORY  = 2,
    EXT2_D_CHARDEV    = 3,
    EXT2_D_BLKDEV     = 4,
    EXT2_D_FIFO       = 5,
    EXT2_D_SOCKET     = 6,
    EXT2_D_LINK       = 7
};

#endif /* ! _SYS_FS_EXT2_ */ 
