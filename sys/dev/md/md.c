/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#include <sys/types.h>
#include <sys/device.h>
#include <sys/utils.h>
#include <sys/kmem.h>
#include <sys/errno.h>
#include <sys/thread.h>
#include <sys/kprintf.h>
#include <sys/uio.h>
#include <dev/md/md.h>
#include <dev/md/md_priv.h>

static d_open_t mdopen;
static d_close_t mdclose;
static d_ioctl_t mdioctl;
static d_read_t mdread;
static d_write_t mdwrite;

static devsw_t md_devsw = {
    mdopen,
    mdclose,
    mdioctl,
    mdread,
    mdwrite,
    nostrategy,
    "md"
};

int
mdopen(devd_t *d, int flags)
{
    memdisk_t *md = d->priv;
    TRACE_IN("d=%p md=%p owner=%p", d, md, md->owner);
    if (md->owner) return -EBUSY;
    md->owner = curthread;
    return 0;
}

int
mdclose(devd_t *d)
{
    memdisk_t *md = d->priv;
    md->owner = NULL;
    return 0;
}

int
mdread(devd_t *d, uio_t *uio)
{
    memdisk_t *md = d->priv;
    TRACE_IN("d=%p md=%p uio=%p", d, md, uio);
    if (md->size < uio->offset ) return -EFAULT;
    size_t rlen = MIN(uio->size, md->size - uio->offset);
    if (uio_copy((char*)md->data + uio->offset, uio, rlen) == -1) {
        return -EIO;
    }
    return rlen;
}

int
mdwrite(devd_t *d, uio_t *uio)
{
    memdisk_t *md = d->priv;
    TRACE_IN("d=%p md=%p uio=%p", d, md, uio);
    if (md->size < uio->offset ) return -EFAULT;
    size_t rlen = MIN(uio->size, md->size - uio->offset);
    if (uio_copy((char*)md->data + uio->offset, uio, rlen) == -1) {
        return -EIO;
    }
    return rlen;
}

int
mdioctl(devd_t *dev, int cmd, uintptr_t arg)
{
    return -ENOTTY;
}

/*=========================================================================
 * Zewnêtrzne procedury.
 */

static bool md_less(const memdisk_t *, const memdisk_t *);
static bool md_is_this(const memdisk_t *, uintptr_t unit);
static list_t memdisks;

bool
md_less(const memdisk_t *mdA, const memdisk_t *mdB)
{
    return mdA->unit < mdB->unit;
}

bool
md_is_this(const memdisk_t *md, uintptr_t unit)
{
    return (md->unit == unit);
}

/**
 * Tworzy "pamiêciowy" dysk twardy.
 * @param unit jednostka dysku (/dev/mdXX) 
 * @param data obraz, gdy NULL to sam alokuje dane
 * @param size wielko¶c w bajtach.
 */
int
md_create(int unit, void *data, size_t size)
{
    const char *_str_type;
    memdisk_t *md = NULL;
    bool findunit = (unit==-1);
    while ( (list_next(&memdisks, md)) ) {
        if (findunit) {
            if (unit == md->unit) {
                unit++;
            }
        } else {
            if (md->unit > unit) break;
            if (md->unit == unit) return -1;
        }
    }
    md = kmem_alloc(sizeof(memdisk_t), KM_SLEEP);
    md->unit = unit;
    md->size = size;
    if (data == NULL) {
        md->data_type = MD_DATA_TYPE_ALLOCATED;
        md->data = kmem_alloc(size, KM_SLEEP);
        _str_type = "alloc";
    } else {
        md->data_type = MD_DATA_TYPE_FOREIGN;
        md->data = data;
        _str_type = "foreign";
    }
    list_insert_in_order(&memdisks, md, (list_less_f*) md_less);
    md->devd = devd_create(&md_devsw, md->unit, md, 
        "memory disk (%s) <%p+%u>",  _str_type, md->data, md->size);
    md->owner = NULL;       
    return 0;
}

void
md_destroy(int unit)
{
    memdisk_t *md = list_find(&memdisks, (list_pred_f*) md_is_this,
        (void*)unit);
    KASSERT(md != NULL);
    KASSERT(md->owner == NULL);
    list_remove(&memdisks, md);
    if (md->data_type == MD_DATA_TYPE_FOREIGN) {
        // kern_free(md->data);
    }
    // kern_free(md);
}

void
md_init()
{
    list_create(&memdisks, offsetof(memdisk_t,L_memdisks), FALSE);
}
