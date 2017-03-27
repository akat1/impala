/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE"
 * If we meet some day, and you think this stuff is worth it, you can buy us 
 * a beer in return. - AUTHORS
 * ----------------------------------------------------------------------------
 */

#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/bio.h>
#include <sys/vfs.h>
#include <sys/vfs/vfs_gen.h>
#include <fs/ext2fs/ext2.h>
#include <fs/ext2fs/ext2_private.h>

/*
 * XXX:
 * need huge refactor
 */

void ext2fs_init(void);
static int ext2fs_mount(vfs_t *fs);
static int ext2fs_unmount(vfs_t *fs);
static vnode_t *ext2fs_getroot(vfs_t *fs);

vfs_ops_t ext2fs_ops = {
    .vfs_mount = ext2fs_mount,
    .vfs_unmount = ext2fs_unmount,
    .vfs_getroot = ext2fs_getroot
};

void
ext2fs_init()
{
    vfs_register("ext2fs", &ext2fs_ops);
}


static int
ext2fs_mount(vfs_t *vfs)
{
    devd_t *dev = vfs->vfs_mdev;
    iobuf_t *bp;
    vnode_t *vroot;
    ext2fs_t *ext2fs;
    ext2fs_superblock_t *ext2sb;
    int error;

    /* read superblock */
    bp = bio_read(dev, 2, EXT2_SUPERBLOCK_SIZE);
    if (ISSET(bp->flags, BIO_READ)) {
        DEBUGF("read superblock has failed");
        error = bp->errno;
        bio_release(bp);
        return -error;
    }

    /* parse superblock */
    ext2sb = (ext2fs_superblock_t *)bp->addr;

    /* check if we deal with ext2 */
    if (ext2sb->sb_magic != EXT2_MAGIC) {
        DEBUGF("wrong magic: %x should be: %x", ext2sb->sb_magic,
          EXT2_MAGIC);
        bio_release(bp);
        return -EINVAL;
    }

    ext2fs = kmem_zalloc((sizeof *ext2fs), KM_SLEEP);
    ext2fs->vfs = vfs;
    ext2fs->blocks_count = ext2sb->sb_blocks_count;
    ext2fs->inodes_count = ext2sb->sb_inodes_count;
    ext2fs->block_size = 0x400 << (ext2sb->sb_log_block_size);
    ext2fs->inodes_per_group = ext2sb->sb_inodesb_per_group;
    ext2fs->blocks_per_group = ext2sb->sb_inodesb_per_group;
    ext2fs->frags_per_group = ext2sb->sb_fragsb_per_group;
    ext2fs->inode_size = ext2sb->sb_inode_size;
    ext2fs->first_data_block = ext2sb->sb_first_data_block;
    
    bio_release(bp);

    DEBUGF("ext2 found:\nblocks: %x  inodes: %x block size: %x",
        ext2fs->blocks_count, ext2fs->inodes_count, ext2fs->block_size);
    DEBUGF("ext2 first data block: %x", ext2fs->first_data_block);

    /* ceil() */
    ext2fs->groups = (ext2fs->blocks_count-1) / ext2fs->blocks_per_group;
    ext2fs->groups++;
    DEBUGF("ext2 groups: %x", ext2fs->groups);

    /* get group descriptors */
    DEBUGF("group descriptors size: %x", ext2fs->groups *
            sizeof(ext2fs_group_descriptor_t));

    ext2fs->group_descriptors = kmem_zalloc(ext2fs->groups *
      sizeof(ext2fs_group_descriptor_t), KM_SLEEP);

    bp = bio_read(dev, 4, EXT2_SUPERBLOCK_SIZE);
    
    if (ISSET(bp->flags, BIO_READ)) {
        DEBUGF("read group descriptors has failed");
        error = bp->errno;
        bio_release(bp);
        return -error;
    }

    mem_cpy(ext2fs->group_descriptors, bp->addr, ext2fs->groups *
      sizeof(ext2fs_group_descriptor_t));

    bio_release(bp);

    /* create root block */
    vroot = ext2fs_get_vnode(ext2fs, 2, VNODE_TYPE_DIR);
    vroot->v_flags |= VNODE_FLAG_ROOT;
    
    ext2fs->root = vroot;

    vfs->vfs_private = ext2fs;

    return 0;
}

static int
ext2fs_unmount(vfs_t *fs)
{
    /* XXX: MUAHAH */
    return 0;
}

static vnode_t *
ext2fs_getroot(vfs_t *vfs)
{
    ext2fs_t *ext2fs = vfs->vfs_private;
    vref(ext2fs->root);
    return ext2fs->root;
}
