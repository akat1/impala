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
 * BIO PATA - it shouldn't be aware of block size of device
 */
enum {
    
    DEV_BLK_SIZE = 512
};

int ext2fs_sync(vnode_t *v);
int ext2fs_getdents(vnode_t *v, dirent_t *dents, int first, int count);
int ext2fs_access(vnode_t *v, int mode, pcred_t *c);
int ext2fs_symlink(vnode_t *v, char *name, char *dst);
int ext2fs_readlink(vnode_t *v, char *buf, int bsize);
int ext2fs_setattr(vnode_t *v, vattr_t *attr);
int ext2fs_open(vnode_t *vn, int flags, mode_t mode);
int ext2fs_close(vnode_t *v);
int ext2fs_ioctl(vnode_t *v, int cmd, uintptr_t arg);
int ext2fs_seek(vnode_t *v, off_t off);
int ext2fs_read(vnode_t *v, uio_t *u, int flags);
int ext2fs_write(vnode_t *v, uio_t *u, int flags);
int ext2fs_truncate(vnode_t *v, off_t len);
int ext2fs_create(vnode_t *v, vnode_t **vpp, const char *name, vattr_t *attr);
int ext2fs_getattr(vnode_t *v, vattr_t *attr);
int ext2fs_lookup(vnode_t *v, vnode_t **vpp, lkp_state_t *state);
int ext2fs_mkdir(vnode_t *v, vnode_t **vpp, const char *path, vattr_t *attr);
int ext2fs_inactive(vnode_t *v);

vnode_ops_t ext2fs_node_ops = {
    ext2fs_open,
    ext2fs_create,
    ext2fs_close,
    ext2fs_read,
    ext2fs_write,
    ext2fs_ioctl,
    ext2fs_seek,
    ext2fs_truncate,
    ext2fs_getattr,
    ext2fs_setattr,
    ext2fs_lookup,
    ext2fs_mkdir,
    ext2fs_getdents,
    ext2fs_readlink,
    ext2fs_symlink,
    ext2fs_access,
    ext2fs_sync,
    ext2fs_inactive,
    vfs_gen_lock,
    vfs_gen_unlock
};

/* HELPERS */
int
ext2fs_dtype_to_vtype(int exttype);

int
ext2fs_dtype_to_vtype(int exttype)
{
    switch (exttype) {
    case EXT2_D_UNKNOWN:
        return -1;
    case EXT2_D_REG:
        return VNODE_TYPE_REG;
    case EXT2_D_DIRECTORY:
        return VNODE_TYPE_DIR;
    case EXT2_D_CHARDEV:
        return VNODE_TYPE_DEV;
    case EXT2_D_BLKDEV:
        return VNODE_TYPE_DEV;
    case EXT2_D_FIFO:
        return VNODE_TYPE_FIFO;
    case EXT2_D_SOCKET:
        return -1;
    case EXT2_D_LINK:
        return VNODE_TYPE_LNK;
    }

    return -1;
}
vnode_t *
ext2fs_get_vnode(ext2fs_t *fs, uint32_t inode_no, int v_type)
{
    ext2fs_node_t *node = NULL;
    vnode_t *v;

    ext2fs_read_node(fs, inode_no, &node);
    v = vnode_alloc();
    v->v_vfs = node->fs->vfs;
    v->v_ops = &ext2fs_node_ops;
    v->v_type = v_type;
    v->v_private = node;

    return v;
}

/* XXX: common */
int
pc_cmp(lkp_state_t *path, const char *fname);

int
pc_cmp(lkp_state_t *path, const char *fname)
{
    const char *c = path->now;
    while(*c && *c!='/' && *fname)
        if(*(c++)!=*(fname++))
            return 1;
    if(*fname || (*c && *c!='/'))
        return 1;
    return 0;
}

int
ext2fs_lookup(vnode_t *v, vnode_t **vpp, lkp_state_t *path)
{
    ext2fs_node_t *node = v->v_private;
    ext2fs_node_dir_t *cur;

    *vpp = NULL;

    cur = list_head(&(node->dirnodes));
    while (cur != NULL) {
        if (pc_cmp(path, cur->dirnode.name) == 0) {
            *vpp = ext2fs_get_vnode(node->fs, cur->dirnode.inode_no,
                    ext2fs_dtype_to_vtype(cur->dirnode.type));
            path->now += str_len(cur->dirnode.name);
            return 0;
        }
        cur = list_next(&(node->dirnodes), cur);
    } 

    return -ENOENT;
}

int
ext2fs_inactive(vnode_t *v)
{
    return 0;
}

int
ext2fs_getdents(vnode_t *v, dirent_t *dents, int first, int count) {

    ext2fs_node_t *node = v->v_private;
    int records = 0;
    ext2fs_node_dir_t *cur;

    if  (v->v_type != VNODE_TYPE_DIR)
        return -ENOTDIR;

    count /= sizeof(dirent_t);

    cur = list_head(&(node->dirnodes));
    while (cur != NULL && count > 0) {
        if (first == 0) {
            dents->d_ino = cur->dirnode.inode_no;
            /* XXX: truncate name */
            str_ncpy(dents->d_name, cur->dirnode.name, sizeof(dents->d_name));
            dents++;
            count--;
            records += sizeof(dirent_t);
        } else
            first--;

        cur = list_next(&(node->dirnodes), cur);
    }

    return records;
}

int
ext2fs_mkdir(vnode_t *v, vnode_t **vpp, const char *path,
vattr_t *attr) {
    return 0;
}

int
ext2fs_truncate(vnode_t *v, off_t len) {
    return 0;
}

int
ext2fs_getattr(vnode_t *v, vattr_t *attr) {
    ext2fs_node_t *node = v->v_private;
    ext2fs_inode_t *inode = node->inode;

    if ( ISSET(attr->va_mask, VATTR_UID) )
        attr->va_uid = inode->uid;
    if ( ISSET(attr->va_mask, VATTR_GID) )
        attr->va_gid = inode->gid;
    if ( ISSET(attr->va_mask, VATTR_MODE) ) {
        attr->va_mode = 0;
        
        /* others */
        if (ISSET(inode->mode, EXT2_S_IXOTH))
            attr->va_mode |= 0001;
        if (ISSET(inode->mode, EXT2_S_IWOTH))
            attr->va_mode |= 0002;
        if (ISSET(inode->mode, EXT2_S_IROTH))
            attr->va_mode |= 0004;
    
        /* others */
        if (ISSET(inode->mode, EXT2_S_IXGRP))
            attr->va_mode |= 0010;
        if (ISSET(inode->mode, EXT2_S_IWGRP))
            attr->va_mode |= 0020;
        if (ISSET(inode->mode, EXT2_S_IRGRP))
            attr->va_mode |= 0040;
    
        /* others */
        if (ISSET(inode->mode, EXT2_S_IXUSR))
            attr->va_mode |= 0100;
        if (ISSET(inode->mode, EXT2_S_IWUSR))
            attr->va_mode |= 0200;
        if (ISSET(inode->mode, EXT2_S_IRUSR))
            attr->va_mode |= 0400;
    
        /* sticky */
        if (ISSET(inode->mode, EXT2_S_ISVTX))
            attr->va_mode |= 01000;
        if (ISSET(inode->mode, EXT2_S_ISGID))
            attr->va_mode |= 02000;
        if (ISSET(inode->mode, EXT2_S_ISUID))
            attr->va_mode |= 04000;
    }
    if ( ISSET(attr->va_mask, VATTR_SIZE) )
        attr->va_size = inode->size;
    if ( ISSET(attr->va_mask, VATTR_TYPE) )
        attr->va_type = v->v_type;
    if ( ISSET(attr->va_mask, VATTR_NLINK) )
        attr->va_nlink = inode->links_count;
    if ( ISSET(attr->va_mask, VATTR_INO) ) {
        attr->va_ino = node->inode_no;
    }

    if ( ISSET(attr->va_mask, VATTR_BLK) ) {
        /* XXX */
        attr->va_blksize = 0;
        attr->va_blocks = 0;
    }
    
    return 0;
}

int
ext2fs_create(vnode_t *v, vnode_t **vpp, const char *name,
vattr_t *attr) {
    return 0;
}

int
ext2fs_write(vnode_t *v, uio_t *u, int flags) {
    return 0;
}

int
ext2fs_read(vnode_t *v, uio_t *u, int flags) {
    ext2fs_node_t *node = v->v_private;
    ext2fs_inode_t *inode = node->inode;
    off_t off = u->offset % (node->fs->block_size);
    uint8_t buf[node->fs->block_size];
    int block = 0;

    if (u->offset >= inode->size)
        u->resid = 0;

    u->completed = 0;
    while (off < inode->size && u->resid > 0) {
        block = u->offset / node->fs->block_size;
        ext2fs_read_from_inode(node->fs, node->inode, buf, block, 1);
        uio_move(buf + off, MIN(node->fs->block_size - off, u->resid), u);
        off = 0;
    }

    return u->completed;
}


off_t ext2fs_block_offset(ext2fs_t *fs, uint32_t group, uint32_t block);

off_t
ext2fs_block_offset(ext2fs_t *fs, uint32_t group, uint32_t block)
{
    return (
        (group * fs->block_size * fs->blocks_per_group) +
        ((block - 1) * fs->block_size) + 
        EXT2_SUPERBLOCK_OFFSET
    );
}

int
ext2fs_read_block(ext2fs_t *fs, uint32_t group, uint32_t block, uint8_t *buf);

int
ext2fs_read_block(ext2fs_t *fs, uint32_t group, uint32_t block, uint8_t *buf)
{
    iobuf_t *bp;
    devd_t *dev = fs->vfs->vfs_mdev;
    int error;

    bp = bio_read(dev, ext2fs_block_offset(fs, group, block) / DEV_BLK_SIZE,
      fs->block_size);

    if (ISSET(bp->flags, BIO_READ)) {
        error = bp->errno;
        bio_release(bp);
        return -error;
    }

    mem_cpy(buf, bp->addr, fs->block_size);

    bio_release(bp);

    return 0;
}

/* block here is offset block */
int
ext2fs_read_from_inode(ext2fs_t *fs, ext2fs_inode_t *inode, uint8_t *buf,
    uint32_t block, uint32_t count)
{
    uint32_t blocks[fs->block_size / sizeof(uint32_t)];
    /* uint32_t group = (inode_no - 1) / fs->inodes_per_group; */
    uint32_t group = 0;

    while (count--) {
        if (block <= EXT2_BLOCK_MAX_DIRECT) {
            ext2fs_read_block(fs, group, inode->block[block], buf);
        } else

        /* XXX: make it efficient */
        /* mapped in first indirect block */
        if (block < EXT2_BLOCK_MAX_DIRECT + (fs->block_size /
                    sizeof(uint32_t)))
        {
            ext2fs_read_block(fs, 0, inode->block[EXT2_BLOCK_INDIRECT],
              (void *)blocks);
            block -= EXT2_BLOCK_INDIRECT;
            ext2fs_read_block(fs, 0, blocks[block], buf);
        } else {
            DEBUGF("XXX: fix me please!");
            mem_set(buf, 0, fs->block_size);
        }

        buf += fs->block_size;
        block++;
    }

    return 0;
}

int
ext2fs_read_node(ext2fs_t *fs, uint32_t inode_no, ext2fs_node_t **node)
{
    uint32_t group;
    uint32_t inode_offset;
    uint32_t block;
    uint32_t block_offset;
    ext2fs_inode_t *inode;
    uint8_t buf[fs->block_size];

    /* calculate group that contains particular inode */
    group = (inode_no - 1) / fs->inodes_per_group;

    /* offset in inodes table */
    inode_offset = (inode_no - 1) % fs->inodes_per_group;

    /* get inode table block for group */
    block = fs->group_descriptors[group].inode_table;

    /* calculate block offset */
    block += inode_offset / (fs->block_size / sizeof(ext2fs_inode_t));
    block_offset = (inode_offset) % (fs->block_size / sizeof(ext2fs_inode_t));
    
    ext2fs_read_block(fs, group, block, buf);
    inode = kmem_zalloc(sizeof(ext2fs_inode_t), KM_SLEEP);
    mem_cpy(inode, &((ext2fs_inode_t *)buf)[block_offset],
            sizeof(ext2fs_inode_t));
    
    *node = kmem_zalloc(sizeof(ext2fs_node_t), KM_SLEEP);
    (*node)->inode_no = inode_no;
    (*node)->inode = inode;
    (*node)->fs = fs;

    /* parse dir */
    if ((inode->mode & EXT2_S_DIR) != 0) {
        uint8_t buf[fs->block_size];
        uint8_t *cur;
        size_t left;
        ext2fs_dirnode_t *dirnode;
        ext2fs_node_dir_t *nodedir;
        
        LIST_CREATE(&(*node)->dirnodes, ext2fs_node_dir_t, L_dirnodes, FALSE);
        /* parse */
        left = inode->size;
        block = 0;
        while (left) {
            /* read next block */
            if (left % fs->block_size == 0) {
                ext2fs_read_from_inode(fs, inode, buf, block++, 1);
                cur = buf;
            }
            dirnode = (ext2fs_dirnode_t *)cur;
            left -= dirnode->entry_size;
            cur += dirnode->entry_size;
            if (dirnode->inode_no == 0) {
                break;
            }
            nodedir = kmem_zalloc(sizeof(ext2fs_node_dir_t), KM_SLEEP);
            mem_cpy(&(nodedir->dirnode), dirnode, sizeof(ext2fs_dirnode_t));
            nodedir->dirnode.name[nodedir->dirnode.length] = '\0';
            list_insert_tail(&(*node)->dirnodes, nodedir);
        }
    }

    return 0;
}

int
ext2fs_open(vnode_t *vn, int flags, mode_t mode)
{
    return 0;
}

int
ext2fs_close(vnode_t *v)
{
    return 0;
}


int
ext2fs_ioctl(vnode_t *v, int cmd, uintptr_t arg)
{
    return -EINVAL;
}

int
ext2fs_seek(vnode_t *v, off_t off)
{
    return -ENOTSUP;
}


int
ext2fs_setattr(vnode_t *v, vattr_t *attr)
{
    DEBUGF("setattr not supported");
    return -ENOTSUP;
}

int
ext2fs_readlink(vnode_t *v, char *buf, int bsize)
{
    return -ENOTSUP;
}

int
ext2fs_symlink(vnode_t *v, char *name, char *dst)
{
    return -ENOTSUP;
}

int
ext2fs_access(vnode_t *v, int mode, pcred_t *c)
{
    return 0;
}

int
ext2fs_sync(vnode_t *v)
{
    return -EOK;
}
