
#include "io.h"

#include <linux/blkdev.h>
#include <linux/buffer_head.h>
#include <linux/bio.h>
#include <linux/log2.h>
#include <linux/hash.h>
#include <linux/blkdev.h>   // blkdev_get_no_open, blkdev_put, block_device struct
#include <linux/fs.h>       // for FMODE_READ, FMODE_WRITE
#include <linux/namei.h>    // kern_path, LOOKUP_FOLLOW
#include <linux/slab.h>     // kmalloc, kcalloc, kfree
#include <linux/version.h>
#include <linux/module.h>
// Don't forget to free the block device with bdev_fput

/*-----------------------------------------------------------------------------
 * Buffer Hash
 *
 * Used to map buffer ID's to KrBuf instances
 * We can tell if a buffer is already cached by looking it up in this table
 *-----------------------------------------------------------------------------
 */
#define WRITE_FLUSH_FUA (REQ_OP_WRITE | REQ_SYNC | REQ_OP_FLUSH | REQ_FUA)

// this function computes the bucket for a given block number
// where to look for the block in the hash table
static __always_inline u32 kr_bufhash_bucket(KrDevice* dev, kr_block block)
{
    return block % dev->maxbufs;
    //return hash_64(block, 32) % dev->maxbufs;
}

// we find a buffer in the hash table
// in the buffer we can have many blocks 
static KrBuf* kr_bufhash_find(KrDevice* dev, kr_block block)
{
    KrBuf* buf = dev->bufhash[kr_bufhash_bucket(dev, block)];
    while (buf) {
        if (buf->block == block)
            return buf;
        buf = buf->next;
    }
    return NULL;
}

// we add a buf in the hash table, at the beginning of the bucket 
static void kr_bufhash_insert(KrBuf* buf)
{
    KrBuf* first = buf->dev->bufhash[buf->bucket];
    if (first)
        first->prev = buf;
    buf->next = first;
    buf->dev->bufhash[buf->bucket] = buf;
}

// we delete a buf from the hash table
static void kr_bufhash_del(KrBuf* buf)
{
    KrBuf** bucket = &buf->dev->bufhash[buf->bucket];
    if (buf->prev)  
        buf->prev->next = buf->next;
    if (buf->next)
        buf->next->prev = buf->prev;

    if (*bucket == buf) /* this buf is the first one in this bucket */
        *bucket = buf->next;

    buf->prev = buf->next = NULL;
}

/*-----------------------------------------------------------------------------
 * I/O Requests
 *
 * Functions to issue Linux bio requests for reading and writing disk blocks.
 * Entry points are _ and _.
 *-----------------------------------------------------------------------------
 */

#define KR_BUF_SECTOR(b) ((b->block)*(KR_BLOCK_SIZE/512))

static struct bio* kr_create_bio(KrDevice* dev, struct page* page, int sector)
{
    struct bio* bio;
    int ret;
    
    printk("Creating bio for sector %d\n", sector);
    
    // CRITICAL FIX: Pass the block device to bio_alloc
    // This properly initializes block cgroup context for loop devices
    bio = bio_alloc(dev->bdev, 1, REQ_OP_WRITE, GFP_NOIO);
    if (!bio) {
        printk(KERN_ERR "kr_create_bio: bio_alloc failed\n");
        return NULL;
    }
    
    // Set the sector
    bio->bi_iter.bi_sector = sector;
    
    // The device is already set by bio_alloc, but ensure it's correct
    // bio_set_dev(bio, dev->bdev); // This is redundant but safe
    
    // Verify the device is set properly
    if (bio->bi_bdev == NULL) {
        printk(KERN_ERR "bio->bi_bdev is NULL after bio_alloc\n");
        bio_put(bio);
        return NULL;
    }
    
    // Add the page and CHECK the return value
    ret = bio_add_page(bio, page, KR_BLOCK_SIZE, 0);
    if (ret != KR_BLOCK_SIZE) {
        printk(KERN_ERR "kr_create_bio: bio_add_page failed, added %d bytes instead of %d\n", 
               ret, KR_BLOCK_SIZE);
        bio_put(bio);
        return NULL;
    }
    
    printk("Successfully created bio for sector %d\n", sector);
    return bio;
}
/**
 * Callback given to bio requests, it will be called when the request completes.
 * See issue_bio_sync below.
 */
static void finish_bio_sync(struct bio *bio)
{
    struct completion *event = bio->bi_private;
    complete(event);
}

/**
 * This is synchronous, so we use a struct completion to block
 * until the request fnishes. The completion will be finished by the callback
 * finish_bio_sync
 * It is synchronous because it waits for the I/O to complete before returning.
 */
static int issue_bio_sync(KrDevice* dev, struct page* page, int sector, int rw)
{
    struct completion event;
    struct bio* bio = kr_create_bio(dev, page, sector);
    if (!bio)
        return -KR_ENOMEM;

    bio->bi_end_io = finish_bio_sync;
    bio->bi_private = &event;

    init_completion(&event);
    submit_bio( bio);
    wait_for_completion(&event);

    bio_put(bio);
    return 0;
}

static void finish_bio_async(struct bio* bio)
{
    bio_put(bio);
}

static int issue_bio_async(KrDevice* dev, struct page* page, int sector, int rw)
{
    struct bio* bio = kr_create_bio(dev, page, sector);

    if (!bio)
        return -KR_ENOMEM;

    bio->bi_end_io = finish_bio_async;
    bio->bi_opf = rw | REQ_SYNC;
    submit_bio(bio);

    return 0;
}

// because i am using /dev/loop which is virtual block device , i need to make sure to charge the data in .img
void kr_buf_writeback(KrBuf* buf) {
    if (!buf )
        return;
    issue_bio_async(buf->dev, buf->page, KR_BUF_SECTOR(buf), WRITE);
}


/*-----------------------------------------------------------------------------
 * Buffer Manager
 *
 *-----------------------------------------------------------------------------
 */

static __always_inline void kr_buf_maybe_write(KrBuf* buf)
{
    if (kr_buf_isdirty(buf)) {
        issue_bio_async(buf->dev, buf->page, KR_BUF_SECTOR(buf), WRITE_FLUSH_FUA);
        buf->flags &= ~KR_BUF_DIRTY; /* clear dirty bit */
    }
}

/**
 * Find an unpinned buf suitable for replacement
 * For now, just evicts the first available buffer it sees.
 */
static KrBuf* kr_buf_evict(KrDevice* dev)
{
    size_t i;
    KrBuf* buf = NULL;

    for (i = 0; i < dev->maxbufs; i++) {
        buf = dev->bufhash[i];
        while (buf) {
            if (buf->pincnt == 0)
                goto found;
            buf = buf->next;
        }
    }

    /* no unpinned bufs! */
    return NULL;

 found:
    kr_bufhash_del(buf);
    kr_buf_maybe_write(buf);
    return buf;
}

// we recuperate a buffer from the cache, or allocate a new one if it doesn't exist
// If read is true, we will read the data from disk into the buffer
// If read is false, we will not read the data from disk, just allocate the buffer
// The buffer is pinned (pincnt is incremented) so it won't be evicted until unpinned
KrBuf* kr_buf_get(KrDevice* dev, kr_block loc, bool read)
{
    KrBuf* buf = kr_bufhash_find(dev, loc);

    /* it's in the cache already: just increase the pincount */
    if (buf) {
        dev->n_hit++;
        buf->pincnt++;
        return buf;
    }

    if (dev->bufcnt >= dev->maxbufs) {
        dev->n_evict++;
        /* cache is already at max size: must evict another unused buffer */
        buf = kr_buf_evict(dev);
        if (!buf)
            return NULL;
    } else {
        /* if the cache isn't at max size yet, allocate a new buffer */
        buf = kzalloc(sizeof(KrBuf), GFP_KERNEL);
        if (!buf)
            return NULL;
        buf->page = alloc_pages(GFP_KERNEL, KR_PAGE_ALLOC_ORDER);
        buf->data = page_address(buf->page);
    }

    buf->block = loc;
    buf->pincnt = 1;
    buf->dev = dev;
    buf->bucket = kr_bufhash_bucket(dev, loc);

    kr_bufhash_insert(buf);

    dev->bufcnt++;

    if (read) {
        dev->n_read++;
        issue_bio_sync(dev, buf->page, KR_BUF_SECTOR(buf), READ);
        
    }

    return buf;
}

void kr_buf_free(KrBuf* buf)
{
    __free_pages(buf->page, KR_PAGE_ALLOC_ORDER);
    kfree(buf);
}

/*-----------------------------------------------------------------------------
 * KrDevice
 *
 * Implementation
 *-----------------------------------------------------------------------------
 */

// Create a new KrDevice instance
// This function opens a block device at the given path and initializes a KrDevice structure
// It returns NULL on failure, or a pointer to the new KrDevice on success
// The cachesz parameter specifies the maximum number of buffers in the cache


KrDevice* kr_device_create(const char* path, size_t cachesz)
{
    dev_t dev_num;
    struct block_device* bdev;
    KrDevice* dev;
    fmode_t mode = FMODE_READ | FMODE_WRITE;
    
    // 1. Obtenir le numéro de périphérique
    if (lookup_bdev(path, &dev_num)) {
        printk(KERN_ERR "kr_device_create: lookup_bdev failed for %s\n", path);
        return NULL;
    }
    
    // 2. Obtenir la structure block_device
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,8,0)
    struct file *bdev_file = bdev_file_open_by_dev(dev_num, mode, NULL, NULL);
    if (IS_ERR(bdev_file)) {
        printk(KERN_ERR "kr_device_create: bdev_file_open_by_dev failed\n");
        return NULL;
    }
    bdev = file_bdev(bdev_file);
#else
    bdev = blkdev_get_by_dev(dev_num, mode, NULL);
    if (IS_ERR(bdev)) {
        printk(KERN_ERR "kr_device_create: blkdev_get_by_dev failed\n");
        return NULL;
    }
#endif
    if (IS_ERR(bdev)) {
        printk(KERN_ERR "kr_device_create: block device open failed\n");
        return NULL;
    }
    
    // 3. Allouer la structure KrDevice
    dev = kmalloc(sizeof(KrDevice), GFP_KERNEL);
    if (!dev) {
        goto err_release_bdev;
    }
    
    dev->bdev = bdev;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,8,0)
    dev->bdev_file = bdev_file;  // Store the file handle too
#endif
    dev->maxbufs = cachesz;
    dev->bufcnt = 0;
    dev->bufhash = kcalloc(dev->maxbufs, sizeof(KrBuf*), GFP_KERNEL);
    if (!dev->bufhash) {
        kfree(dev);
        goto err_release_bdev;
    }
    
    dev->n_evict = dev->n_read = dev->n_hit = dev->n_dbl = 0;
    return dev;

err_release_bdev:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,8,0)
    bdev_fput(bdev_file);
#else
    blkdev_put(bdev, mode);
#endif
    return NULL;
}

void kr_device_release(KrDevice* dev)
{
    int i;
    KrBuf *buf, *tmp;

    printk(KERN_INFO "evict %u read %u hit %u dbl %u\n",
        dev->n_evict, dev->n_read, dev->n_hit, dev->n_dbl);

    /* write dirty bufs, and free all allocated bufs */
    for (i = 0; i < dev->maxbufs; i++) {
        buf = dev->bufhash[i];
        while (buf) {
            /* we're freeing buf and can't access buf->next after we do */
            tmp = buf->next;
            kr_buf_maybe_write(buf);
            kr_buf_free(buf);
            buf = tmp;
        }
    }

    // bdev_fput(dev->bdev, FMODE_READ | FMODE_WRITE);
    kfree(dev->bufhash);
    kfree(dev);
}

void kr_device_flush(KrDevice* dev)
{
    int i;
    KrBuf* buf;

    for (i = 0; i < dev->maxbufs; i++) {
        buf = dev->bufhash[i];
        while (buf) {
            kr_buf_maybe_write(buf);
            buf = buf->next;
        }
    }
}

EXPORT_SYMBOL(kr_device_create);
EXPORT_SYMBOL(kr_device_release);
EXPORT_SYMBOL(kr_device_flush);

MODULE_LICENSE("GPL");