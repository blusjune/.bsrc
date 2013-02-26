/*
 * tier : Tiered storage made easy.
 *        Tier allows to create a virtual blockdevice
 *        that consists of multiple physical devices.
 *        A common configuration would be to use
 *        SSD/SAS/SATA 
 *
 * Partly based up-on sbd and the loop driver.
 * Redistributable under the terms of the GNU GPL.
 * Author: Mark Ruijter, mruijter@gmail.com
 */
#include "btier.h"
#include "btier_main.h"
#include "btier_common.h"

#define TRUE 1
#define FALSE 0
#define TIER_VERSION "0.9.9.8"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mark Ruijter");

LIST_HEAD(device_list);
static DEFINE_MUTEX(tier_devices_mutex);

/*
 * The internal representation of our device.
 */
static struct tier_device *device = NULL;
static char *devicenames;
static struct mutex ioctl_mutex;

#include "btier_sysfs.c"

/*
 * The device operations structure.
 */
static struct block_device_operations tier_ops = {
	.owner = THIS_MODULE,
};

static struct attribute_group tier_attribute_group = {
	.name = "tier",
	.attrs = tier_attrs,
};

static int tier_sysfs_init(struct tier_device *dev)
{
	int res;
	res = sysfs_create_group(&disk_to_dev(dev->gd)->kobj,
				 &tier_attribute_group);
	return (res);
}

static void tier_sysfs_exit(struct tier_device *dev)
{
	sysfs_remove_group(&disk_to_dev(dev->gd)->kobj, &tier_attribute_group);
}

static struct devicemagic *read_device_magic(struct tier_device *dev, int count)
{
	struct devicemagic *dmagic;

	dmagic = kzalloc(sizeof(struct devicemagic), GFP_KERNEL);
	tier_file_read(dev,count, (char *)dmagic,
		       sizeof(struct devicemagic), 0);
	return dmagic;
}

static void write_device_magic(struct tier_device *dev, int count)
{
	tier_file_write(dev, count,
			(char *)dev->backdev[count]->devmagic,
			sizeof(struct devicemagic), 0);
}

static void mark_device_clean(struct tier_device *dev, int count)
{
	dev->backdev[count]->devmagic->clean = CLEAN;
	memset(&dev->backdev[count]->devmagic->binfo_journal_new, 0,
	       sizeof(struct blockinfo));
	memset(&dev->backdev[count]->devmagic->binfo_journal_old, 0,
	       sizeof(struct blockinfo));
	write_device_magic(dev, count);
}

static int mark_offset_as_used(struct tier_device *dev, int devicenr,
			       u64 offset)
{
	u64 boffset;
	u8 allocated = 0xff;
	int ret;

        boffset = offset >>BLKBITS;
	ret = tier_file_write(dev, devicenr, &allocated, 1,
			      dev->backdev[devicenr]->startofbitlist + boffset);
	if (NULL != dev->backdev[devicenr]->bitlist)
		dev->backdev[devicenr]->bitlist[boffset] = allocated;
	return ret;
}

static void clear_dev_list(struct tier_device *dev, struct blockinfo *binfo)
{
	u64 offset;
	u64 boffset;
	u8 unallocated = 0x00;

	offset = binfo->offset - dev->backdev[binfo->device - 1]->startofdata;

        boffset = offset >>BLKBITS;
	tier_file_write(dev, binfo->device - 1,
			&unallocated, 1,
			dev->backdev[binfo->device - 1]->startofbitlist +
			boffset);
	if (dev->backdev[binfo->device - 1]->free_offset > boffset)
		dev->backdev[binfo->device - 1]->free_offset = boffset;
	if (NULL != dev->backdev[binfo->device - 1]->bitlist)
                memset(&dev->backdev[binfo->device - 1]->bitlist[boffset],0,1);
}

static int allocate_dev(struct tier_device *dev, u64 blocknr,
			struct blockinfo *binfo, int devicenr)
{
	char *buffer = NULL;
	unsigned int buffercount;
	u64 cur = 0;
	u64 relative_offset = 0;
	int ret = 0;

        cur = dev->backdev[devicenr]->free_offset >> 12;
	/* The bitlist may be loaded into memory or be NULL if not */
	if (NULL == dev->backdev[devicenr]->bitlist)
		buffer = kzalloc(PAGE_SIZE, GFP_KERNEL);
	while (0 == binfo->device
	       && (cur * PAGE_SIZE) < dev->backdev[devicenr]->bitlistsize) {
		if (NULL == dev->backdev[devicenr]->bitlist) {
			tier_file_read(dev,devicenr,
				       buffer, PAGE_SIZE,
				       dev->backdev[devicenr]->startofbitlist +
				       (cur * PAGE_SIZE));
		} else
			buffer =
			    &dev->backdev[devicenr]->bitlist[cur * PAGE_SIZE];
		buffercount = 0;
		while (0 == binfo->device) {
			if (0xff != (0xff & buffer[buffercount])) {
				binfo->offset =
				    (cur * PAGE_SIZE * BLKSIZE) +
				    (buffercount * BLKSIZE);
				relative_offset = binfo->offset;
				binfo->offset +=
				    dev->backdev[devicenr]->startofdata;
				if (binfo->offset + BLKSIZE >
				    dev->backdev[devicenr]->endofdata) {
					/* We are at the end of this device, no space available */
					goto end_exit;
				} else {
					binfo->device = devicenr + 1;
					ret = mark_offset_as_used(dev, devicenr,
								  relative_offset);
					if (0 != ret)
						goto end_exit;
                                        dev->backdev[devicenr]->free_offset = relative_offset >> BLKBITS;
					dev->backdev[devicenr]->usedoffset =
					    binfo->offset;
				}
			}
			buffercount++;
			if (buffercount >= PAGE_SIZE)
				break;
		}
		cur++;
	}
      end_exit:
	if (NULL == dev->backdev[devicenr]->bitlist)
		kfree(buffer);
	return ret;
}

static int allocate_block(struct tier_device *dev, u64 blocknr,
			  struct blockinfo *binfo)
{
	int devicenr = 0;
	int count = 0;

/* Sequential writes will go to SAS or SATA */
	if (dev->iotype == SEQUENTIAL && dev->attached_devices > 1)
		devicenr = dev->sequential_landing;
	while (1) {
		if (0 != allocate_dev(dev, blocknr, binfo, devicenr))
			return -EIO;
		if (0 != binfo->device) {
			if (0 != write_blocklist(dev, blocknr, binfo, WT))
				return -EIO;
			break;
		}
		devicenr++;
		count++;
		if (count >= dev->attached_devices) {
			TIERERR
			    ("no free space found, this should never happen!!");
			return -ENOSPC;
		}
		if (devicenr >= dev->attached_devices)
			devicenr = 0;
	}
	return 0;
}

/**
 * tier_file_write - helper for writing data
 */
static int tier_file_write(struct tier_device *dev, unsigned int devicenr,
			   u8 * buf, const int len, loff_t pos)
{
	ssize_t bw;
	mm_segment_t old_fs = get_fs();

	set_fs(get_ds());
	bw = dev->backdev[devicenr]->tier_dta_file->f_op->write(dev->backdev
								[devicenr]->
								tier_dta_file,
								buf, len, &pos);
	dev->backdev[devicenr]->shouldsync = 1;
	set_fs(old_fs);
	if (likely(bw == len))
		return 0;
	TIERERR("Write error on device %s at offset %llu, length %i.\n",
		dev->backdev[devicenr]->tier_dta_file->f_dentry->d_name.name,
		(unsigned long long)pos, len);
	if (bw >= 0)
		bw = -EIO;
	return bw;
}

static int read_tiered(struct tier_device *dev, char *data,
		       unsigned int len, u64 offset)
{
	struct blockinfo *binfo = NULL;
	u64 blocknr;
	unsigned int block_offset;
	int res = 0;
	int size = 0;
	unsigned int done = 0;
        u64 curoff;

	if (dev->iotype == RANDOM)
		dev->stats.rand_reads++;
	else
		dev->stats.seq_reads++;
	if (len == 0)
		return -1;
	while (done < len) {
                curoff=offset+done;
                blocknr = curoff >> BLKBITS;
		block_offset = curoff - (blocknr << BLKBITS);

		binfo = get_blockinfo(dev, blocknr, TIERREAD);
                if ( dev->inerror ) {
                   res=-EIO;
                   break;
                }
		if (len - done + block_offset > BLKSIZE) {
			size = BLKSIZE - block_offset;
		} else
			size = len - done;
		if (0 == binfo->device) {
			memset(data + done, 0, size);
			res = 0;
		} else {
			res =
			    tier_file_read(dev,binfo->device - 1,
					   data + done, size,
					   binfo->offset + block_offset);
		}
		kfree(binfo);
		done += size;
		if (res != 0)
			break;
	}
	return res;
}

static int write_tiered(struct tier_device *dev, char *data, unsigned int len,
			u64 offset)
{
	struct blockinfo *binfo;
	u64 blocknr;
	unsigned int block_offset;
	int res = 0;
	int domig;
	unsigned int size = 0;
	unsigned int done = 0;
        u64 curoff;

	if (dev->iotype == RANDOM)
		dev->stats.rand_writes++;
	else
		dev->stats.seq_writes++;
	while (done < len) {
                curoff=offset+done;
                blocknr = curoff >> BLKBITS;
		block_offset = curoff - (blocknr << BLKBITS );
		binfo = get_blockinfo(dev, blocknr, TIERWRITE);
                if ( dev->inerror) {
                   res=-EIO;
                   break;
                }
		if (0 == binfo->device) {
			res = allocate_block(dev, blocknr, binfo);
			domig = 0;
			if (res != 0) {
				TIERCRIT("Failed to allocate_block");
				return res;
			}
		} else
			domig = 1;
		if (len - done + block_offset > BLKSIZE) {
			size = BLKSIZE - block_offset;
		} else
			size = len - done;
		res =
		    tier_file_write(dev, binfo->device - 1,
				    data + done, size,
				    binfo->offset + block_offset);
		kfree(binfo);
		done += size;
		if (res != 0)
			break;
	}
	return res;
}

/**
 * tier_file_read - helper for reading data
 */
static int tier_file_read(struct tier_device *dev,unsigned int device,
			  u8 * buf, const int len, loff_t pos)
{
        struct file *file=dev->backdev[device]->tier_dta_file;
	ssize_t bw;
	mm_segment_t old_fs = get_fs();
        /* Disable readahead on random IO */
        if (dev->iotype == RANDOM)
              file->f_ra.ra_pages=0;
        else file->f_ra.ra_pages=dev->backdev[device]->ra_pages;
	set_fs(get_ds());
	bw = file->f_op->read(file, buf, len, &pos);
	set_fs(old_fs);
	if (likely(bw == len))
		return 0;
	TIERERR("Read error at byte offset %llu, length %i.\n",
		(unsigned long long)pos, len);
	if (bw >= 0)
		bw = -EIO;
	return bw;
}

static int tier_sync(struct tier_device *dev)
{
	int ret = 0;
	int count;

	for (count = 0; count < dev->attached_devices; count++) {
		if (dev->backdev[count]->shouldsync) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
			ret = vfs_fsync(dev->backdev[count]->tier_dta_file, 0);
#else
			ret =
			    vfs_fsync(dev->backdev[count]->tier_dta_file,
				      dev->backdev[count]->tier_dta_file->
				      f_path.dentry, 0);
#endif
			if (ret != 0)
				break;
			dev->backdev[count]->shouldsync = 0;
		}
	}
	return ret;
}

static void *as_sprintf(const char *fmt, ...)
{
	/* Guess we need no more than 100 bytes. */
	int n, size = 100;
	void *p;
	va_list ap;
	p = kmalloc(size, GFP_ATOMIC);
	while (1) {
		/* Try to print in the allocated space. */
		va_start(ap, fmt);
		n = vsnprintf(p, size, fmt, ap);
		va_end(ap);
		/* If that worked, return the string. */
		if (n > -1 && n < size)
			return p;
		/* Else try again with more space. */
		if (n > -1)	/* glibc 2.1 */
			size = n + 1;	/* precisely what is needed */
		else		/* glibc 2.0 */
			size *= 2;	/* twice the old size */
		p = krealloc(p, size, GFP_ATOMIC);
	}
}

static char *as_strarrcat(const char **strarr, ssize_t count)
{
	int totallen = 0;
	int i;
	char *retstr = NULL, *curpos;

	for (i = 0; i < count; i++) {
		totallen += strlen(strarr[i]);
	}

	curpos = retstr = kzalloc(totallen + 1, GFP_KERNEL);
	for (i = 0; i < count; i++) {
		strcpy(curpos, strarr[i]);
		curpos += strlen(strarr[i]);
	}

	return retstr;
}

static void tiererror(struct tier_device *dev, char *msg)
{
       dev->inerror=1;
       TIERCRIT("tiererror : %s",msg);
}

/* Delayed metadata update routine */
static void update_blocklist(struct tier_device *dev, u64 blocknr, struct blockinfo *binfo)
{
       struct blockinfo *odinfo;
       int res;

       odinfo = kzalloc(sizeof(struct blockinfo), GFP_KERNEL);
       if (NULL == odinfo)
                tiererror(dev,"kzalloc failed");
       if ( dev->inerror ) return;
       res = tier_file_read(dev,0,
                           (char *)odinfo, sizeof(struct blockinfo),
                            dev->backdev[0]->startofblocklist +
                           (blocknr * sizeof(struct blockinfo)));
       if (res != 0)
            tiererror(dev,"tier_file_read : returned an error");
       if ( 0 != memcmp(binfo,odinfo,sizeof(struct blockinfo)) ) {
           //TIERDEBUG("delayed flush of blockinfo from blocknr %llu",blocknr);
           (void)write_blocklist(dev,blocknr,binfo, WD);
       }
       kfree(odinfo);  
}

/* Check for corruption */
static int binfo_sanity(struct tier_device *dev,struct blockinfo *binfo)
{
    if (binfo->device > dev->attached_devices) {
       TIERINFO("Metadata corruption detected : device %u, dev->attached_devices %u",
           binfo->device,dev->attached_devices);
       tiererror(dev,"get_blockinfo : binfo->device > dev->attached_devices");
       kfree(binfo);
       return 0;
    }

    if (binfo->offset > dev->backdev[binfo->device - 1]->devicesize) {
       TIERINFO("Metadata corruption detected : device %u, offset %llu, devsize %llu",
          binfo->device,binfo->offset,dev->backdev[binfo->device - 1]->devicesize);
       tiererror(dev,"get_blockinfo : offset exceeds device size");
       kfree(binfo);
       return 0;
    }
    return 1;
}

/* Read the metadata of the blocknr specified
   When a blocknr is not yet allocated binfo->device is 0
   otherwhise > 0 
   Metadata is modified when called with TIERREAD */
static struct blockinfo *get_blockinfo(struct tier_device *dev, u64 blocknr,
				       int updatemeta)
{
/* The blocklist starts at the end of the bitlist on device1 */
	struct blockinfo *binfo;
	int res;

	binfo = kzalloc(sizeof(struct blockinfo), GFP_KERNEL);
	if (NULL == binfo)
		tiererror(dev,"kzalloc failed");
        if ( dev->inerror ) return NULL;
	if (dev->backdev[0]->blocklist != NULL) {
		memcpy(binfo, dev->backdev[0]->blocklist[blocknr],
		       sizeof(struct blockinfo));
	} else {

		res = tier_file_read(dev,0,
				     (char *)binfo, sizeof(struct blockinfo),
				     dev->backdev[0]->startofblocklist +
				     (blocknr * sizeof(struct blockinfo)));
		if (res != 0) {
			tiererror(dev,"tier_file_read : returned an error");
                        kfree(binfo);
                        return NULL;
                }
	}

	if (0 != binfo->device) {
                if ( !binfo_sanity(dev,binfo)) return NULL;

/* update accesstime and hitcount */
		if (updatemeta > 0) {
			if (updatemeta == TIERREAD) {
				if (binfo->readcount < 65536)
					binfo->readcount++;
				dev->backdev[binfo->device -
					     1]->devmagic->total_reads++;
			} else {
				if (binfo->writecount < 65536)
					binfo->writecount++;
				dev->backdev[binfo->device -
					     1]->devmagic->total_writes++;
			}
			binfo->lastused = get_seconds();
                        (void)write_blocklist(dev,blocknr,binfo, WB);
		}
	}
	return binfo;
}

static int write_blocklist(struct tier_device *dev, u64 blocknr,
			   struct blockinfo *binfo, int write_policy)
{
	int ret=0;
	u64 blocklist_offset = dev->backdev[0]->startofblocklist;

	blocklist_offset += (blocknr * sizeof(struct blockinfo));
	binfo->lastused = get_seconds();
        if ( write_policy != WD ) {
	     if (NULL != dev->backdev[0]->blocklist)
	     	memcpy(dev->backdev[0]->blocklist[blocknr], binfo,
	     	       sizeof(struct blockinfo));
        }
        if ( write_policy == WT ) {
	   ret =
	       tier_file_write(dev, 0, (char *)binfo, sizeof(struct blockinfo),
	   		    blocklist_offset);
	   if (ret != 0)
		TIERCRIT("write_blocklist failed to write blockinfo");
        }
	return ret;
}

static void sync_device(struct tier_device *dev, int count)
{
	if (dev->backdev[count]->shouldsync) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
		vfs_fsync(dev->backdev[count]->tier_dta_file, 0);
#else
		vfs_fsync(dev->backdev[count]->tier_dta_file,
			  dev->backdev[count]->tier_dta_file->f_path.dentry, 0);
#endif
		dev->backdev[count]->shouldsync = 0;
	}
}

static void write_blocklist_journal(struct tier_device *dev, u64 blocknr,
				    struct blockinfo *newdevice,
				    struct blockinfo *olddevice)
{
	memcpy(&dev->backdev[olddevice->device - 1]->
	       devmagic->binfo_journal_old, olddevice,
	       sizeof(struct blockinfo));
	memcpy(&dev->backdev[olddevice->device - 1]->
	       devmagic->binfo_journal_new, newdevice,
	       sizeof(struct blockinfo));
	dev->backdev[olddevice->device - 1]->devmagic->blocknr_journal =
	    blocknr;
	tier_file_write(dev, olddevice->device - 1,
			(char *)dev->backdev[olddevice->device - 1]->devmagic,
			sizeof(struct devicemagic), 0);
	sync_device(dev, olddevice->device - 1);
}

static void clean_blocklist_journal(struct tier_device *dev, int device)
{
	memset(&dev->backdev[device]->devmagic->binfo_journal_old, 0,
	       sizeof(struct blockinfo));
	memset(&dev->backdev[device]->devmagic->binfo_journal_new, 0,
	       sizeof(struct blockinfo));
	dev->backdev[device]->devmagic->blocknr_journal = 0;
	tier_file_write(dev, device,
			(char *)dev->backdev[device]->devmagic,
			sizeof(struct devicemagic), 0);
	sync_device(dev, device);
}

static void recover_journal(struct tier_device *dev, int device)
{
	u64 blocknr;

	tier_file_read(dev,device,
		       (char *)dev->backdev[device]->devmagic,
		       sizeof(struct devicemagic), 0);
	if (0 == dev->backdev[device]->devmagic->binfo_journal_old.device) {
		TIERINFO
		    ("recover_journal : journal is clean, no need to recover");
		return;
	}
	blocknr = dev->backdev[device]->devmagic->blocknr_journal;
	write_blocklist(dev, blocknr,
			&dev->backdev[device]->devmagic->binfo_journal_old, WT);
	if (0 != dev->backdev[device]->devmagic->binfo_journal_new.device)
		clear_dev_list(dev,
			       &dev->backdev[device]->
			       devmagic->binfo_journal_new);
	clean_blocklist_journal(dev, device);
	TIERINFO
	    ("recover_journal : recovered pending migration of blocknr %llu",
	     blocknr);
}

/*
 * Grab first pending buffer
 */
static struct bio *tier_get_bio(struct tier_device *dev)
{
	return bio_list_pop(&dev->tier_bio_list);
}

static void determine_iotype(struct tier_device *dev, u64 blocknr)
{
	int ioswitch = 0;
	if (blocknr >= dev->lastblocknr && blocknr <= dev->lastblocknr + 1) {
		ioswitch = 1;
	}
	if (ioswitch && dev->insequence < 10)
		dev->insequence++;
	else {
		if (dev->insequence > 0)
			dev->insequence--;
	}
	if (dev->insequence > 5 ) {
		dev->iotype = SEQUENTIAL;
	} else {
		dev->iotype = RANDOM;
	}
	dev->lastblocknr = blocknr;
}

static void discard_on_real_device(struct tier_device *dev,
				   struct blockinfo *binfo)
{
	struct block_device *bdev;
	sector_t sector, nr_sects, endsector;
	u64 endoffset;
	unsigned int sector_size;
	u64 curdevsize;
	u64 devsectors;
	unsigned long flags = 0;

	int ret;
	curdevsize =
	    KERNEL_SECTORSIZE *
	    tier_get_size(dev->backdev[binfo->device - 1]->tier_dta_file);
	bdev =
	    lookup_bdev(dev->backdev[binfo->device - 1]->devmagic->
			fullpathname);
	if (IS_ERR(bdev)) {
		TIERDEBUG("lookup_bdev failed for device %u", binfo->device - 1);
		return;
	}
	devsectors = get_capacity(bdev->bd_disk);
	sector_size = curdevsize / devsectors;

	sector = binfo->offset / sector_size;
	if (sector * sector_size < binfo->offset)
		sector++;

	endoffset = binfo->offset + BLKSIZE;
	endsector = endoffset / sector_size;
	nr_sects = endsector - sector;
	ret = blkdev_issue_discard(bdev, sector, nr_sects, GFP_KERNEL, flags);
	if (0 == ret)
		TIERDEBUG
		    ("discarded : device %s : sector %lu, nrsects %lu,sectorsize %u",
		     dev->backdev[binfo->device - 1]->devmagic->fullpathname,
		     sector, nr_sects, sector_size);
}

/* Reset blockinfo for this block to unused and clear the
   bitlist for this block

   TODO : Pass discard to underlying devices that support it
          like SSD's used in tier0 
*/
static void tier_discard(struct tier_device *dev, u64 offset, unsigned int size)
{
	struct blockinfo *binfo;
	u64 blocknr;
	u64 lastblocknr;
        u64 curoff;
	u64 start;

        curoff = offset + size;
        lastblocknr = curoff >> BLKBITS;
        start = offset >> BLKBITS;
	/* Make sure we don't discard a block while a part of it is still inuse */
	if (start * BLKSIZE < offset)
		start++;

	for (blocknr = start; blocknr < lastblocknr; blocknr++) {
		binfo = get_blockinfo(dev, blocknr, 0);
                if ( dev->inerror ) break;
		if (binfo->device == 0) {
			kfree(binfo);
			continue;
		}
		TIERDEBUG("really discard blocknr %llu at offset %llu size %u",
			  blocknr, offset, size);
		clear_dev_list(dev, binfo);
		reset_counters_on_migration(dev, binfo);
		discard_on_real_device(dev, binfo);
		memset(binfo, 0, sizeof(struct blockinfo));
		write_blocklist(dev, blocknr, binfo, WT);
		kfree(binfo);
	}
}

static int tier_do_bio(struct tier_device *dev, struct bio *bio)
{
	loff_t offset;
	struct bio_vec *bvec;
	int i, ret = 0;
	u64 blocknr = 0;
	char *buffer;
	int locked = 0;

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,0,0)
	const u64 do_sync = (bio->bi_rw & WRITE_SYNC);
#else
	const u64 do_sync = (bio->bi_rw & REQ_SYNC);
#endif

        if ( !atomic_dec_and_test(&dev->wcachelock)) {
		mutex_lock(&dev->cachelock);
		locked = 1;
	}

	offset = ((loff_t) bio->bi_sector << 9);
        blocknr = offset >> BLKBITS;

	if (bio_rw(bio) == WRITE) {
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,0,0)
		if (bio_rw_flagged(bio, BIO_RW_BARRIER)) {
#else
		if (bio->bi_rw & REQ_FLUSH) {
#endif
			if (dev->barrier) {
				ret = tier_sync(dev);
				if (unlikely(ret && ret != -EINVAL)) {
					ret = -EIO;
					goto out;
				}
			}
		}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
		if (bio->bi_rw & REQ_DISCARD) {
			TIERDEBUG("Got a discard request offset %llu len %u",
				  offset, bio->bi_size);
			tier_discard(dev, offset, bio->bi_size);
		}
#endif
	}

	bio_for_each_segment(bvec, bio, i) {
		determine_iotype(dev, blocknr);
		buffer = kmap(bvec->bv_page);
		if (bio_rw(bio) == WRITE) {
			ret = write_tiered(dev,
					   buffer + bvec->bv_offset,
					   bvec->bv_len, offset);
		} else {
			ret = read_tiered(dev,
					  buffer + bvec->bv_offset,
					  bvec->bv_len, offset);
		}
		kunmap(bvec->bv_page);
		if (ret < 0)
			break;
		offset += bvec->bv_len;
                blocknr = offset >> BLKBITS;
	}

	if (bio_rw(bio) == WRITE) {
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,0,0)
		if (bio_rw_flagged(bio, BIO_RW_BARRIER)) {
#else
		if (bio->bi_rw & REQ_FUA) {
#endif
			if (dev->barrier) {
				ret = tier_sync(dev);
				if (unlikely(ret && ret != -EINVAL))
					ret = -EIO;
			}
		}
		if (do_sync && dev->ptsync) {
			ret = tier_sync(dev);
			if (unlikely(ret && ret != -EINVAL))
				ret = -EIO;
		}
	}
      out:
	atomic_inc(&dev->wcachelock);
	if (locked)
		mutex_unlock(&dev->cachelock);
	return ret;
}

static inline void tier_handle_bio(struct tier_device *dev, struct bio *bio)
{
	int ret;
	ret = tier_do_bio(dev, bio);
	bio_endio(bio, ret);
}

static void reset_counters_on_migration(struct tier_device *dev,
					struct blockinfo *binfo)
{
        u64 devblocks=dev->backdev[binfo->device - 1]->devicesize >> BLKBITS;

	if (dev->migrate_verbose) {
		TIERINFO("block %u-%llu reads %u writes %u", binfo->device,
			 binfo->offset, binfo->readcount, binfo->writecount);
		TIERINFO("devmagic->total_writes was %llu",
			 dev->backdev[binfo->device -
				      1]->devmagic->total_writes);
		TIERINFO("devmagic->total_reads was %llu",
			 dev->backdev[binfo->device -
				      1]->devmagic->total_reads);
	}
	dev->backdev[binfo->device - 1]->devmagic->total_reads -=
	    binfo->readcount;
	dev->backdev[binfo->device - 1]->devmagic->total_writes -=
	    binfo->writecount;
        dev->backdev[binfo->device - 1]->devmagic->average_writes =
            dev->backdev[binfo->device - 1]->devmagic->total_writes / devblocks;
        dev->backdev[binfo->device - 1]->devmagic->average_reads = 
            dev->backdev[binfo->device - 1]->devmagic->total_reads / devblocks;
	if (dev->migrate_verbose) {
		TIERINFO("devmagic->total_writes is now %llu",
			 dev->backdev[binfo->device -
				      1]->devmagic->total_writes);
		TIERINFO("devmagic->total_reads is now %llu",
			 dev->backdev[binfo->device -
				      1]->devmagic->total_reads);
	}
	return;
}

/* When a block is migrated to a different tier
   the readcount and writecount are reset to 0.
   The block now has hit_collecttime seconds to
   collect enough hits. After which it is compared
   to the average hits that blocks have had on this
   device. Should the block score less then average
   hits then it will be migrated to an even lower tier.

   Allthough reads and writes are counted seperately
   for now they are threated equally.

   We can in the future differentiate between SLC
   and MLC SSD's and store chunks with high read and
   low write frequency on MLC SSD. And chunks that
   are often re-written on SLC SSD.
*/
static int copyblock(struct tier_device *dev, struct blockinfo *newdevice,
		     struct blockinfo *olddevice, u64 curblock)
{
	int devicenr = newdevice->device - 1;
	char *buffer;

	newdevice->device = 0;
/* reset readcount and writecount up-on migration
   to another tier */
	newdevice->readcount = 0;
	newdevice->writecount = 0;
	newdevice->lastused = get_seconds();
	allocate_dev(dev, curblock, newdevice, devicenr);
	if (0 == newdevice->device) {
		//TIERINFO
		//    ("No free space found on device %u to migrate blocknr %llu",
		//     devicenr, curblock);
		return 0;
	}
	buffer = vzalloc(BLKSIZE);
	tier_file_read(dev,olddevice->device - 1,
		       buffer, BLKSIZE, olddevice->offset);
	tier_file_write(dev, newdevice->device - 1,
			buffer, BLKSIZE, newdevice->offset);
	write_blocklist_journal(dev, curblock, newdevice, olddevice);
	write_blocklist(dev, curblock, newdevice, WT);
	sync_device(dev, newdevice->device - 1);
	clean_blocklist_journal(dev, olddevice->device - 1);
	vfree(buffer);
	if (dev->migrate_verbose)
		TIERINFO
		    ("migrated blocknr %llu from device %u-%llu to device %u-%llu",
		     curblock, olddevice->device - 1, olddevice->offset,
		     newdevice->device - 1, newdevice->offset);
	return 1;
}

static int migrate_up_ifneeded(struct tier_device *dev, struct blockinfo *binfo,
			       u64 curblock)
{
	int res = 0;
	struct blockinfo *orgbinfo;
	u64 hitcount = 0;
	u64 avghitcount = 0;
	u64 avghitcountnexttier = 0;

	if (NULL == binfo)
		return res;
	if (binfo->device <= 1)
		return res;

	orgbinfo = kzalloc(sizeof(struct blockinfo), GFP_KERNEL);
	memcpy(orgbinfo, binfo, sizeof(struct blockinfo));

	hitcount = binfo->readcount + binfo->writecount;
	avghitcount =
	    dev->backdev[binfo->device - 1]->devmagic->average_reads +
	    dev->backdev[binfo->device - 1]->devmagic->average_writes;
	if (hitcount > avghitcount + (avghitcount / dev->attached_devices)) {
		if (binfo->device > 1) {
			avghitcountnexttier =
			    dev->backdev[binfo->device -
					 2]->devmagic->average_reads +
			    dev->backdev[binfo->device -
					 2]->devmagic->average_writes;
			if (hitcount >
			    avghitcountnexttier - (avghitcountnexttier / dev->attached_devices))
				binfo->device--;
		}
	}
	if (orgbinfo->device != binfo->device) {
		//TIERERR
		//    ("migrate_up_ifneeded : binfo->lastused %llu seconds %llu, hitcount %llu avghitcount %llu",
		//    binfo->lastused, seconds, hitcount, avghitcount);
		res = copyblock(dev, binfo, orgbinfo, curblock);
		if (res) {
			reset_counters_on_migration(dev, orgbinfo);
			clear_dev_list(dev, orgbinfo);
			discard_on_real_device(dev, orgbinfo);
		} 
	}
	kfree(orgbinfo);
	return res;
}

static int migrate_down_ifneeded(struct tier_device *dev,
				 struct blockinfo *binfo, u64 curblock)
{
	int res = 0;
	time_t seconds = get_seconds();
	struct blockinfo *orgbinfo;
	u64 hitcount = 0;
	u64 avghitcount = 0;

	if (NULL == binfo)
		return res;
	if (binfo->device == 0)
		return res;

	orgbinfo = kzalloc(sizeof(struct blockinfo), GFP_KERNEL);
	memcpy(orgbinfo, binfo, sizeof(struct blockinfo));

	hitcount = binfo->readcount + binfo->writecount;
	avghitcount =
	    dev->backdev[binfo->device - 1]->devmagic->average_reads +
	    dev->backdev[binfo->device - 1]->devmagic->average_writes;
	if (seconds - binfo->lastused >
	    dev->backdev[binfo->device - 1]->dtapolicy.max_age)
		binfo->device++;
	else if (hitcount < avghitcount - (avghitcount / dev->attached_devices)
		 && seconds - binfo->lastused >
		 dev->backdev[binfo->device - 1]->dtapolicy.hit_collecttime)
		if (binfo->device < dev->attached_devices - 1)
			binfo->device++;
	if (binfo->device > dev->attached_devices) {
		binfo->device = orgbinfo->device;
	} else if (orgbinfo->device != binfo->device) {
		//TIERERR
		//    ("migrate_down_ifneeded : binfo->lastused %llu seconds %llu, hitcount %llu avghitcount %llu",
		//     binfo->lastused, seconds, hitcount, avghitcount);
		res = copyblock(dev, binfo, orgbinfo, curblock);
		if (res) {
			reset_counters_on_migration(dev, orgbinfo);
			clear_dev_list(dev, orgbinfo);
			discard_on_real_device(dev, orgbinfo);
		}
	}
	kfree(orgbinfo);
	return res;
}

static void load_bitlists(struct tier_device *dev)
{
	int device;
	u64 cur;

	for (device = 0; device < dev->attached_devices; device++) {
		dev->backdev[device]->bitlist =
		    vzalloc(dev->backdev[device]->bitlistsize);
		if (NULL == dev->backdev[device]->bitlist) {
			TIERINFO
			    ("Failed to allocate memory to load bitlist %u in memory, proceed without bitlist cache",
			     device);
			break;
		}
		for (cur = 0; cur < dev->backdev[device]->bitlistsize;
		     cur += PAGE_SIZE) {
			tier_file_read(dev,device,
				       &dev->backdev[device]->bitlist[cur],
				       PAGE_SIZE,
				       dev->backdev[device]->startofbitlist +
				       cur);
		}
	}
}

static void free_bitlists(struct tier_device *dev)
{
	int device;

	for (device = 0; device < dev->attached_devices; device++) {
		if (NULL != dev->backdev[device]->bitlist) {
			vfree(dev->backdev[device]->bitlist);
			dev->backdev[device]->bitlist = NULL;
		}
	}
}

static void load_blocklist(struct tier_device *dev)
{
	int alloc_failed = 0;
	u64 curblock;
	u64 blocks = dev->size >> BLKBITS;
	u64 listentries = dev->blocklistsize / sizeof(struct blockinfo);
	int res;
	// We use vmalloc since this can become a large amount of memory
	dev->backdev[0]->blocklist =
	    vzalloc(sizeof(struct blockinfo *) * listentries);
	if (NULL == dev->backdev[0]->blocklist)
		return;
	for (curblock = 0; curblock < blocks; curblock++) {
		dev->backdev[0]->blocklist[curblock] =
		    kzalloc(sizeof(struct blockinfo), GFP_KERNEL);
		if (NULL == dev->backdev[0]->blocklist[curblock]) {
			alloc_failed = 1;
			break;
		}
		res = tier_file_read(dev,0,
				     (char *)dev->backdev[0]->
				     blocklist[curblock],
				     sizeof(struct blockinfo),
				     dev->backdev[0]->startofblocklist +
				     (curblock * sizeof(struct blockinfo)));
		if (res != 0)
			tiererror(dev,"tier_file_read : returned an error");
	}
	if (alloc_failed) {
		free_blocklist(dev);
	}
}

static void free_blocklist(struct tier_device *dev)
{
	u64 curblock;
	u64 blocks = dev->size >> BLKBITS;
	if (NULL == dev->backdev[0]->blocklist)
		return;
	for (curblock = 0; curblock < blocks; curblock++) {
		if (NULL != dev->backdev[0]->blocklist[curblock]) {
			write_blocklist(dev, curblock,
					(struct blockinfo *)dev->backdev[0]->
					blocklist[curblock], WD);
			kfree(dev->backdev[0]->blocklist[curblock]);
		}
	}
	vfree(dev->backdev[0]->blocklist);
	dev->backdev[0]->blocklist = NULL;
}

static void walk_blocklist(struct tier_device *dev)
{
	u64 blocks = dev->size >> BLKBITS;
	u64 curblock;
	struct blockinfo *binfo;
	int interrupted = 0;
	int res = 0;
	int mincount = 0;
        u64 devblocks;

	if (dev->migrate_verbose)
		TIERERR("walk_blocklist start from : %llu",
			dev->resumeblockwalk);
	for (curblock = dev->resumeblockwalk; curblock < blocks; curblock++) {
		if (dev->stop || dev->migration_disabled || dev->inerror )
			break;
		binfo = get_blockinfo(dev, curblock, 0);
                if ( dev->inerror ) break;
		if (binfo->device != 0) {
                        devblocks=dev->backdev[binfo->device - 1]->devicesize >> BLKBITS;
                        dev->backdev[binfo->device - 1]->devmagic->average_reads =
                             dev->backdev[binfo->device - 1]->devmagic->total_reads / devblocks;
                        dev->backdev[binfo->device - 1]->devmagic->average_writes =
                             dev->backdev[binfo->device - 1]->devmagic->total_writes / devblocks;
			res = migrate_down_ifneeded(dev, binfo, curblock);
			if (!res)
				res = migrate_up_ifneeded(dev, binfo, curblock);
                        if (!res) update_blocklist(dev,curblock,binfo);
		}
		kfree(binfo);
		if ( 0 > atomic_read(&dev->wcachelock)) {
			mincount++;
			if (mincount > 5 || res) {
				dev->resumeblockwalk = curblock;
				interrupted = 1;
				break;
			}
		}
	}
        if ( dev->inerror) return;
	tier_sync(dev);
	if (!interrupted) {
		dev->resumeblockwalk = 0;
		dev->migrate_timer.expires =
		    jiffies + msecs_to_jiffies(dev->migration_interval * 1000);
	} else {
		dev->migrate_timer.expires = jiffies + msecs_to_jiffies(3000);
	}
	if (!dev->stop && !dev->migration_disabled)
		add_timer(&dev->migrate_timer);
}

static void data_migrator(struct work_struct *work)
{
	struct tier_device *dev;
	tier_worker_t *mwork;
	mwork = (tier_worker_t *) work;

	dev = mwork->device;
	while (!dev->stop) {
		wait_event_interruptible(dev->migrate_event,
					 1 == atomic_read(&dev->migrate)
					 || dev->stop);
		if (dev->migrate_verbose)
			TIERINFO("data_migrator woke up");
		if (dev->stop)
			break;
                if ( ! atomic_dec_and_test(&dev->wcachelock)){
			if (dev->migrate_verbose)
                            TIERINFO("NORMAL_IO pending: backoff");
			dev->migrate_timer.expires =
			    jiffies + msecs_to_jiffies(3000);
			if (!dev->stop && !dev->migration_disabled)
				mod_timer_pinned(&dev->migrate_timer,
						 dev->migrate_timer.expires);
			atomic_set(&dev->migrate, 0);
                        atomic_inc(&dev->wcachelock);
			continue;
		}
		mutex_lock(&dev->cachelock);
		tier_sync(dev);
		walk_blocklist(dev);
		atomic_set(&dev->migrate, 0);
		mutex_unlock(&dev->cachelock);
		if (dev->migrate_verbose)
			TIERINFO("data_migrator goes back to sleep");
                atomic_inc(&dev->wcachelock);
	}
	kfree(work);
	TIERINFO("data_migrator halted");
}

static int tier_thread(void *data)
{
	struct tier_device *dev = data;
	struct bio *bio;

	set_user_nice(current, -20);

	while (!kthread_should_stop() || !bio_list_empty(&dev->tier_bio_list)) {

		wait_event_interruptible(dev->tier_event,
					 !bio_list_empty(&dev->tier_bio_list) ||
					 kthread_should_stop());
                while ( !bio_list_empty(&dev->tier_bio_list)) {
		    spin_lock_irq(&dev->lock);
		    bio = tier_get_bio(dev);
		    spin_unlock_irq(&dev->lock);
		    BUG_ON(!bio);
		    tier_handle_bio(dev, bio);
                }
	}
	TIERINFO("tier_thread worker halted");
	return 0;
}

static void tier_add_bio(struct tier_device *dev, struct bio *bio)
{
	bio_list_add(&dev->tier_bio_list, bio);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,0)
static int tier_make_request(struct request_queue *q, struct bio *old_bio)
#else
static void tier_make_request(struct request_queue *q, struct bio *old_bio)
#endif
{
	int cpu;
	struct tier_device *dev = q->queuedata;
	int rw = bio_rw(old_bio);

	if (rw == READA)
		rw = READ;

	BUG_ON(!dev || (rw != READ && rw != WRITE));
	spin_lock_irq(&dev->lock);
	if (!dev->active)
		goto out;
	cpu = part_stat_lock();
	part_stat_inc(cpu, &dev->gd->part0, ios[rw]);
	part_stat_add(cpu, &dev->gd->part0, sectors[rw], bio_sectors(old_bio));
	part_stat_unlock();
	tier_add_bio(dev, old_bio);
	wake_up(&dev->tier_event);
	spin_unlock_irq(&dev->lock);
	goto end_return;

      out:
	spin_unlock_irq(&dev->lock);
	bio_io_error(old_bio);

      end_return:
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,0)
	return 0;
#else
	return;
#endif
}

static void init_devicenames(void)
{
	int count;
/* Allow max 24 devices to be configured */
	devicenames = kmalloc(sizeof(char) * 26, GFP_KERNEL);

	for (count = 0; count <= 25; count++) {
		devicenames[count] = 97 + count;
	}
}

static void release_devicename(char *devicename)
{
	int pos;
	char d;

	if (NULL == devicename)
		return;
	d = devicename[6];	/*tierN */
/* Restore the char in devicenames */
	pos = d - 97;
	devicenames[pos] = d;
	kfree(devicename);
}

static char *reserve_devicename(unsigned int *devnr)
{
	char devicenr;
	char *retname;
	int count;
	for (count = 0; count <= 25; count++) {
		devicenr = devicenames[count];
		if (devicenr != 0)
			break;
	}
	if (0 == devicenr) {
		TIERERR("Maximum number of devices exceeded");
		return NULL;
	}
	retname = as_sprintf("sdtier%c", devicenr);
	*devnr = count;
	devicenames[count] = 0;
	return retname;
}

static void migrate_timer_expired(unsigned long q)
{
	struct tier_device *dev = (struct tier_device *)q;

	if (0 == atomic_read(&dev->migrate)) {
		atomic_set(&dev->migrate, 1);
		wake_up(&dev->migrate_event);
	}
}

static void tier_check(struct tier_device *dev, int devicenr)
{
	TIERINFO("device %s is not clean, check forced",
		 dev->backdev[devicenr]->tier_dta_file->f_dentry->d_name.name);
	recover_journal(dev, devicenr);
}

/* Zero out the bitlist starting at offset startofbitlist
   with size bitlistsize */
static void wipe_bitlist(struct tier_device *dev, int device,
			 u64 startofbitlist, u64 bitlistsize)
{
	char *buffer;
	u64 offset = 0;

	//TIERINFO("wipe_bitlist : device %u, start 0x%llx %llu end 0x%llx %llu",
	//          device,startofbitlist,startofbitlist,
	//          bitlistsize+startofbitlist,bitlistsize+startofbitlist);
	buffer = kzalloc(PAGE_SIZE, GFP_KERNEL);
	while (offset < bitlistsize) {
		tier_file_write(dev, device, buffer,
				PAGE_SIZE, startofbitlist + offset);
		offset += PAGE_SIZE;
	}
	if (offset < bitlistsize)
		tier_file_write(dev, device, buffer,
				bitlistsize - offset, startofbitlist + offset);
	kfree(buffer);
}

static u64 allocated_on_device(struct tier_device *dev, int device)
{
	u_char *buffer = NULL;
	u64 offset = 0;
	int count;
	u64 allocated = 0;

	if (NULL == dev->backdev[device]->bitlist)
		buffer = kzalloc(PAGE_SIZE, GFP_KERNEL);
	while (offset < dev->backdev[device]->bitlistsize) {
		if (NULL == dev->backdev[device]->bitlist) {
			tier_file_read(dev,device,
				       buffer, PAGE_SIZE,
				       dev->backdev[device]->startofbitlist +
				       offset);
		} else
			buffer = &dev->backdev[device]->bitlist[offset];
		offset += PAGE_SIZE;
		for (count = 0; count < PAGE_SIZE; count++) {
			if (buffer[count] == 0xff)
				allocated += BLKSIZE;
		}
	}
	if (offset < dev->backdev[device]->bitlistsize) {
		if (NULL == dev->backdev[device]->bitlist) {
			tier_file_read(dev,device,
				       buffer,
				       dev->backdev[device]->bitlistsize -
				       offset,
				       dev->backdev[device]->startofbitlist +
				       offset);
		} else
			buffer = &dev->backdev[device]->bitlist[offset];
	}
	for (count = 0; count < dev->backdev[device]->bitlistsize - offset;
	     count++) {
		if (buffer[count] == 0xff)
			allocated += BLKSIZE;
	}
	if (NULL == dev->backdev[device]->bitlist)
		kfree(buffer);
	return allocated;
}

static void repair_bitlists(struct tier_device *dev)
{
	u64 blocknr;
	struct blockinfo *binfo;
	u64 relative_offset;
	unsigned int count;

	TIERINFO("repair_bitlists : clearing and rebuilding bitlists");
	for (count = 0; count < dev->attached_devices; count++) {
		wipe_bitlist(dev, count,
			     dev->backdev[count]->startofbitlist,
			     dev->backdev[count]->bitlistsize);
		dev->backdev[count]->free_offset = 0;
	}

	for (blocknr = 0; blocknr < dev->size >> BLKBITS; blocknr++) {
		binfo = get_blockinfo(dev, blocknr, 0);
                if ( dev->inerror ) return;
		if (0 == binfo->device)
			continue;
		if (binfo->device > dev->attached_devices) {
			TIERERR
			    ("repair_bitlists : cleared corrupted blocklist entry for blocknr %llu",
			     blocknr);
			memset(binfo, 0, sizeof(struct blockinfo));
			continue;
		}
		if (BLKSIZE + binfo->offset >
		    dev->backdev[binfo->device - 1]->devicesize) {
			TIERERR
			    ("repair_bitlists : cleared corrupted blocklist entry for blocknr %llu",
			     blocknr);
			memset(binfo, 0, sizeof(struct blockinfo));
			continue;
		}
		relative_offset =
		    binfo->offset - dev->backdev[binfo->device -
						 1]->startofdata;
		mark_offset_as_used(dev, binfo->device - 1, relative_offset);
		dev->backdev[count]->free_offset = relative_offset >> BLKBITS;
	}
}

static void order_devices(struct tier_device *dev)
{
	int swap;
	int count;
	int newnr;
	int clean = 1;
	struct backing_device *backdev;

/* Allocate and load */
	backdev = kzalloc(sizeof(struct tier_device), GFP_KERNEL);
	for (count = 0; count < dev->attached_devices; count++) {
		dev->backdev[count]->devmagic = read_device_magic(dev, count);
		if (count != dev->backdev[count]->devmagic->devicenr)
			swap = 1;
	}

/* Check and swap */
	if (swap) {
		for (count = 0; count < dev->attached_devices; count++) {
			dev->backdev[count]->devmagic =
			    read_device_magic(dev, count);
			newnr = dev->backdev[count]->devmagic->devicenr;
			if (count != newnr) {
				memcpy(backdev, dev->backdev[count],
				       sizeof(struct backing_device));
				memcpy(dev->backdev[count], dev->backdev[newnr],
				       sizeof(struct backing_device));
				memcpy(dev->backdev[newnr], backdev,
				       sizeof(struct backing_device));
			}
		}
	}
/* Mark as inuse */
	for (count = 0; count < dev->attached_devices; count++) {
		if (CLEAN != dev->backdev[count]->devmagic->clean) {
			tier_check(dev, count);
			clean = 0;
		}
		dev->backdev[count]->devmagic->clean = DIRTY;
		write_device_magic(dev, count);
		TIERINFO("device %s registered as tier %u",
			 dev->backdev[count]->tier_dta_file->f_dentry->
			 d_name.name, count);
		dev->backdev[count]->dtapolicy.max_age = TIERMAXAGE;
		dev->backdev[count]->dtapolicy.hit_collecttime =
		    TIERHITCOLLECTTIME;
	}
	if (!clean)
		repair_bitlists(dev);
	kfree(backdev);
}

static void register_new_device_size(struct tier_device *dev)
{

	dev->nsectors = dev->size / dev->logical_block_size;
	dev->size = dev->nsectors * dev->logical_block_size;
	set_capacity(dev->gd, dev->nsectors * (dev->logical_block_size / 512));
	revalidate_disk(dev->gd);
	/* let user-space know about the new size */
	kobject_uevent(&disk_to_dev(dev->gd)->kobj, KOBJ_CHANGE);
}

static int tier_register(struct tier_device *dev)
{
	int devnr;
	int ret = 0;
	tier_worker_t *migrateworker;

	dev->devname = reserve_devicename(&devnr);
	if (NULL == dev->devname)
		return -1;
	dev->active = 1;
	dev->barrier = 1;
	dev->sequential_landing = 1;
	dev->migration_interval = MIGRATE_INTERVAL;
/* Barriers can not be used when we work in ram only */
	if (0 == dev->logical_block_size)
		dev->logical_block_size = 512;
        if ( dev->logical_block_size != 512 && 
             dev->logical_block_size != 1024 && 
             dev->logical_block_size != 2048 && 
             dev->logical_block_size != 4096 ) dev->logical_block_size=512;
        if ( dev->logical_block_size == 512 ) dev->nsectors = dev->size >> 9;
        if ( dev->logical_block_size == 1024 ) dev->nsectors = dev->size >> 10;
        if ( dev->logical_block_size == 2048 ) dev->nsectors = dev->size >> 11;
        if ( dev->logical_block_size == 4096 ) dev->nsectors = dev->size >> 12;
	dev->size = dev->nsectors * dev->logical_block_size;
	TIERINFO("%s size : %llu", dev->devname, dev->size);
	spin_lock_init(&dev->lock);
	bio_list_init(&dev->tier_bio_list);
	dev->rqueue = blk_alloc_queue(GFP_KERNEL);
	if (!dev->rqueue) {
		ret = -ENOMEM;
		goto out;
	}
	init_waitqueue_head(&dev->tier_event);
	init_waitqueue_head(&dev->migrate_event);
	dev->cacheentries = 0;
	dev->migrate_verbose = 0;
	dev->stop = 0;
	dev->iotype = RANDOM;
	atomic_set(&dev->migrate, 0);
	atomic_set(&dev->commit, 0);
	atomic_set(&dev->wcachelock, 1);
	load_blocklist(dev);
	load_bitlists(dev);
	/*
	 * Get a request queue.
	 */
	mutex_init(&dev->tier_ctl_mutex);
	mutex_init(&dev->cachelock);
	/*
	 * set queue make_request_fn, and add limits based on lower level
	 * device
	 */
	blk_queue_make_request(dev->rqueue, tier_make_request);
	dev->rqueue->queuedata = (void *)dev;

	/* Tell the block layer that we are not a rotational device
	   and that we support discard aka trim.
	 */
	queue_flag_set_unlocked(QUEUE_FLAG_DISCARD, dev->rqueue);
	blk_queue_logical_block_size(dev->rqueue, dev->logical_block_size);
	blk_queue_io_opt(dev->rqueue, BLKSIZE);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
	if (dev->barrier)
		blk_queue_flush(dev->rqueue, REQ_FLUSH | REQ_FUA);
#endif
	/*
	 * Get registered.
	 */
	dev->major_num = register_blkdev(0, dev->devname);
	if (dev->major_num <= 0) {
		printk(KERN_WARNING "tier: unable to get major number\n");
		goto out;
	}
	/*
	 * And the gendisk structure.
	 */

	/* We support 256 (kernel default) partitions */
	dev->gd = alloc_disk(DISK_MAX_PARTS);
	if (!dev->gd)
		goto out_unregister;
	dev->gd->major = dev->major_num;
	dev->gd->first_minor = 0;
	dev->gd->fops = &tier_ops;
	dev->gd->private_data = &dev;
	strcpy(dev->gd->disk_name, dev->devname);
	set_capacity(dev->gd, dev->nsectors * (dev->logical_block_size / 512));
	dev->gd->queue = dev->rqueue;
	dev->tier_thread = kthread_create(tier_thread, dev, dev->devname);
	if (IS_ERR(dev->tier_thread)) {
		TIERERR("Failed to create kernel thread");
		ret = PTR_ERR(dev->tier_thread);
		goto out_unregister;
	}
	wake_up_process(dev->tier_thread);
	migrateworker = kzalloc(sizeof(tier_worker_t), GFP_KERNEL);
	if (!migrateworker) {
		TIERERR("Failed to allocate memory for migrateworker");
		goto out_unregister;
	}
	migrateworker->device = dev;
	dev->managername = as_sprintf("%s-manager", dev->devname);
	dev->migration_queue = create_workqueue(dev->managername);
	INIT_WORK((struct work_struct *)migrateworker, data_migrator);
	queue_work(dev->migration_queue, (struct work_struct *)migrateworker);
	init_timer(&dev->migrate_timer);
	dev->migrate_timer.data = (unsigned long) dev;
	dev->migrate_timer.function = migrate_timer_expired;
	dev->migrate_timer.expires =
	    jiffies + msecs_to_jiffies(dev->migration_interval * 1000);
	add_timer(&dev->migrate_timer);
	add_disk(dev->gd);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)
	blk_queue_max_discard_sectors(dev->rqueue, get_capacity(dev->gd));
	dev->rqueue->limits.discard_granularity = BLKSIZE;
	dev->rqueue->limits.discard_alignment = BLKSIZE;
#endif
	tier_sysfs_init(dev);
	/* let user-space know about the new size */
	kobject_uevent(&disk_to_dev(dev->gd)->kobj, KOBJ_CHANGE);
	return ret;

      out_unregister:
	unregister_blkdev(dev->major_num, dev->devname);
      out:
	return ret;
}

static loff_t tier_get_size(struct file *file)
{
	loff_t size;

	// Compute loopsize in bytes 
	size = i_size_read(file->f_mapping->host);
	// *
	// * Unfortunately, if we want to do I/O on the device,
	// * the number of 512-byte sectors has to fit into a sector_t.
	// *
	return size >> 9;
}

static int tier_set_fd(struct tier_device *tier, unsigned int arg, int fdnr)
{
	int error = -EBADF;
	struct file *file;

	file = fget(arg);
	if (!file)
		goto out;
	if (!(file->f_mode & FMODE_WRITE)) {
		error = -EPERM;
		goto out;
	}
	error = 0;
	tier->backdev[fdnr]->tier_dta_file = file;
        /* btier disables readahead when it detects a random io pattern
           it restores the original when the pattern becomes sequential */
        tier->backdev[fdnr]->ra_pages=file->f_ra.ra_pages; 
      out:
	return error;
}

/* Return the number of devices in nr
   and return the last tier_device */
static struct tier_device *device_nr(int *nr)
{
	struct list_head *pos, *q;
	struct tier_device *ret = NULL;

	*nr = 0;
	list_for_each_safe(pos, q, &device_list) {
		ret = list_entry(pos, struct tier_device, list);
		*nr += 1;
	}
	return ret;
}

static int tier_device_count(void)
{
	struct list_head *pos, *q;
	int count = 0;

	list_for_each_safe(pos, q, &device_list) {
		count++;
	}
	return count;
}

static void tier_deregister(struct tier_device *tier)
{
	int count;
	if (tier->active) {
		tier->stop = 1;
		wake_up(&tier->migrate_event);
		destroy_workqueue(tier->migration_queue);
		kthread_stop(tier->tier_thread);
		tier_sysfs_exit(tier);
		mutex_destroy(&tier->tier_ctl_mutex);
		mutex_destroy(&tier->cachelock);
		del_timer_sync(&tier->migrate_timer);
		del_gendisk(tier->gd);
		put_disk(tier->gd);
		blk_cleanup_queue(tier->rqueue);
		TIERINFO("deregister device %s", tier->devname);
		unregister_blkdev(tier->major_num, tier->devname);
		list_del(&tier->list);
		kfree(tier->managername);
		release_devicename(tier->devname);
		tier_sync(tier);
		free_blocklist(tier);
		free_bitlists(tier);
		for (count = 0; count < tier->attached_devices; count++) {
			mark_device_clean(tier, count);
			filp_close(tier->backdev[count]->tier_dta_file, NULL);
			kfree(tier->backdev[count]->devmagic);
			kfree(tier->backdev[count]);
		}
		kfree(tier->backdev);
		kfree(tier);
		tier = NULL;
	}
}

static void del_tier_device(char *devicename)
{
	struct tier_device *tier, *next;

	list_for_each_entry_safe(tier, next, &device_list, list) {
		if (NULL != tier->devname) {
			if (NULL != strstr(devicename, tier->devname)) {
				tier_deregister(tier);
			}
		}
	}
}

static int determine_device_size(struct tier_device *dev)
{
	int count;
	dev->size = dev->backdev[0]->devmagic->total_device_size;
	dev->backdev[0]->startofblocklist =
	    dev->backdev[0]->devmagic->startofblocklist;
	dev->blocklistsize = dev->backdev[0]->devmagic->blocklistsize;
	TIERINFO("dev->blocklistsize               : 0x%llx (%llu)",
		 dev->blocklistsize, dev->blocklistsize);
	dev->backdev[0]->endofdata = dev->backdev[0]->startofblocklist - 1;
	for (count = 0; count < dev->attached_devices; count++) {
		dev->backdev[count]->bitlistsize =
		    dev->backdev[count]->devmagic->bitlistsize;
		dev->backdev[count]->startofdata = TIER_HEADERSIZE;
		dev->backdev[count]->startofbitlist =
		    dev->backdev[count]->devmagic->startofbitlist;
		dev->backdev[count]->devicesize =
		    dev->backdev[count]->devmagic->devicesize;
		if (count > 0) {
			dev->backdev[count]->endofdata =
			    dev->backdev[count]->startofbitlist - 1;
		}
		TIERINFO("dev->backdev[%i]->devicesize      : 0x%llx (%llu)",
			 count, dev->backdev[count]->devicesize,
			 dev->backdev[count]->devicesize);
		TIERINFO("dev->backdev[%i]->startofdata     : 0x%llx", count,
			 dev->backdev[count]->startofdata);
		TIERINFO("dev->backdev[%i]->bitlistsize     : 0x%llx", count,
			 dev->backdev[count]->bitlistsize);
		TIERINFO("dev->backdev[%i]->startofbitlist  : 0x%llx", count,
			 dev->backdev[count]->startofbitlist);
		TIERINFO("dev->backdev[%i]->endofdata       : 0x%llx", count,
			 dev->backdev[count]->endofdata);

	}
	TIERINFO("dev->backdev[0]->startofblocklist: 0x%llx",
		 dev->backdev[0]->startofblocklist);
	return 0;
}

static u64 calc_new_devsize(struct tier_device *dev, int cdev, u64 curdevsize)
{
	int count;
	u64 devsize = 0;
	unsigned int header_size = TIER_HEADERSIZE;

	for (count = 0; count < dev->attached_devices; count++) {
		if (cdev == count) {
			devsize +=
			    curdevsize - TIER_DEVICE_PLAYGROUND - header_size;
			continue;
		}
		devsize +=
		    dev->backdev[count]->devicesize - TIER_DEVICE_PLAYGROUND;
	}
	return devsize;
}

static u64 new_total_bitlistsize(struct tier_device *dev, int cdev,
				 u64 curbitlistsize)
{
	int count;
	u64 bitlistsize = 0;

	for (count = 0; count < dev->attached_devices; count++) {
		if (cdev == count) {
			bitlistsize += curbitlistsize;
			continue;
		}
		bitlistsize += dev->backdev[count]->bitlistsize;
	}
	return bitlistsize;
}

/* Copy a list from one location to another
   Return : 0 on success -1 on error  */
static int copylist(struct tier_device *dev, int devicenr,
		    u64 ostart, u64 osize, u64 nstart)
{
	int res = 0;
	u64 offset;
	u64 newoffset = nstart;
	char *buffer;

	TIERINFO
	    ("copylist device %u, ostart 0x%llx (%llu) osize  0x%llx (%llu), nstart 0x%llx (%llu) end 0x%llx (%llu)",
	     devicenr, ostart, ostart, osize, osize, nstart, nstart,
	     nstart + osize, nstart + osize);
	buffer = kzalloc(PAGE_SIZE, GFP_KERNEL);
	for (offset = ostart; offset < ostart + osize; offset += PAGE_SIZE) {
		//TIERINFO("offset = %llu, newoffset=%llu",offset, newoffset);
		res =
		    tier_file_read(dev,devicenr,
				   buffer, PAGE_SIZE, offset);
		if (res < 0)
			break;
		res =
		    tier_file_write(dev, devicenr,
				    buffer, PAGE_SIZE, newoffset);
		if (res < 0)
			break;
		newoffset += PAGE_SIZE;
	}
	if (offset - ostart < osize) {
		TIERINFO
		    ("copylist has failed, not expanding : offset %llu, ostart %llu, osize %llu",
		     offset, ostart, osize);
		res = -1;
	}
	kfree(buffer);
	return res;
}

/* migrate a bitlist from one location to another
   Afterwards changes the structures to point to the new bitlist
   so that the old bitlist location is no longer used
   Return : 0 on success, negative on error */
static int migrate_bitlist(struct tier_device *dev, int devicenr,
			   u64 newdevsize,
			   u64 newbitlistsize, u64 newstartofbitlist)
{
	int res = 0;

	TIERINFO("migrate_bitlist : device %u", devicenr);
	if (newstartofbitlist < dev->backdev[devicenr]->devicesize) {
		TIERINFO("Device size has not grown enough to expand");
		return -1;
	}
	wipe_bitlist(dev, devicenr, newstartofbitlist, newbitlistsize);
	res =
	    copylist(dev, devicenr, dev->backdev[devicenr]->startofbitlist,
		     dev->backdev[devicenr]->bitlistsize, newstartofbitlist);
	if (res != 0)
		return res;
	// Make sure the new bitlist is synced to disk before
	// we continue
	if (0 != (res = tier_sync(dev)))
		return res;
	return res;
}

/* When the blocklist needs to be expanded 
   we have to move blocks of data out of the way
   then expand the bitlist and migrate it from it's
   current location to the new location.
   Since the blocklist is growing tier device 0
   will shrink in usable size. Therefore the bitlist
   may shrink as well. However to reduce complexity
   we let it be for now. */
static int migrate_data_if_needed(struct tier_device *dev, u64 startofblocklist,
				  u64 blocklistsize, int changeddevice)
{
	int res = 0;
	int cbres = 0;
	u64 blocks = dev->size >> BLKBITS;
	u64 curblock;
	struct blockinfo *binfo;
	struct blockinfo *orgbinfo;

	TIERINFO("migrate_data_if_needed");
	binfo = kzalloc(sizeof(struct blockinfo), GFP_KERNEL);
	for (curblock = 0; curblock < blocks; curblock++) {
		/* Do not update the blocks metadata */
		orgbinfo = get_blockinfo(dev, curblock, 0);
                if ( dev->inerror ) {
                   res=-EIO;
                   break;
                }
		// Migrating blocks from device 0 + 1;
		if (orgbinfo->device != 1) {
			kfree(orgbinfo);
			continue;
		}
		cbres = 1;
		TIERINFO("migrate_data_if_needed : blocknr %llu from device %u",
			 curblock, orgbinfo->device - 1);
		if (orgbinfo->offset >= startofblocklist
		    && orgbinfo->offset <= startofblocklist + blocklistsize) {
			memcpy(binfo, orgbinfo, sizeof(struct blockinfo));
			// Move the block to the device that has grown
			binfo->device = changeddevice + 1;
			TIERINFO
			    ("Call copyblock blocknr %llu from device %u to device %u",
			     curblock, orgbinfo->device - 1, binfo->device - 1);
			cbres = copyblock(dev, binfo, orgbinfo, curblock);
			if (cbres) {
				reset_counters_on_migration(dev, orgbinfo);
				clear_dev_list(dev, orgbinfo);
			} else
				TIERINFO
				    ("Failed to migrate blocknr %llu from device %u to device %u",
				     curblock, orgbinfo->device - 1,
				     binfo->device - 1);
		}
		kfree(orgbinfo);
		if (!cbres) {
			res = -1;
			break;
		}
	}
	kfree(binfo);
	TIERINFO("migrate_data_if_needed return %u", res);
	return res;
}

static int do_resize_tier(struct tier_device *dev, int devicenr,
			  u64 newdevsize,
			  u64 newblocklistsize,
			  u64 newbitlistsize, u64 curdevsize)
{
	int res = 0;
	u64 startofnewblocklist;
	u64 startofnewbitlist;

	TIERINFO("resize device %s devicenr %u from %llu to %llu",
		 dev->backdev[devicenr]->tier_dta_file->f_dentry->d_name.name,
		 devicenr, dev->backdev[devicenr]->devicesize, curdevsize);
	startofnewbitlist = newdevsize - newbitlistsize;
	res =
	    migrate_bitlist(dev, devicenr, newdevsize, newbitlistsize,
			    startofnewbitlist);
	if (0 != res)
		return res;
	/* When device 0 has grown we move the bitlist of the device to
	   the end of the device and then move the blocklist to the end
	   This does not require data migration 

	   When another device has grown we may need to expand the blocklist
	   on device 0 as well. In that case we may need to migrate data
	   from device0 to another device to make room for the larger 
	   blocklist */
	if (devicenr == 0) {
		startofnewblocklist = startofnewbitlist - newblocklistsize;
		wipe_bitlist(dev, devicenr, startofnewblocklist,
			     newblocklistsize);
		res =
		    copylist(dev, devicenr,
			     dev->backdev[devicenr]->startofblocklist,
			     dev->blocklistsize, startofnewblocklist);
		if (0 != res)
			return res;
		if (0 != (res = tier_sync(dev)))
			return res;
		dev->backdev[devicenr]->startofblocklist = startofnewblocklist;
		dev->blocklistsize = newblocklistsize;
		dev->backdev[devicenr]->devmagic->blocklistsize =
		    newblocklistsize;
		dev->backdev[devicenr]->devmagic->startofblocklist =
		    startofnewblocklist;
	} else {
		startofnewblocklist =
		    dev->backdev[0]->startofbitlist - newblocklistsize;
		if (startofnewblocklist < dev->backdev[0]->startofblocklist) {
			res =
			    migrate_data_if_needed(dev, startofnewblocklist,
						   newblocklistsize, devicenr);
			if (0 != res)
				return res;
// This should be journalled. FIX FIX FIX
// The blocklist needs to be protected at all cost.
			res =
			    copylist(dev, 0, dev->backdev[0]->startofblocklist,
				     dev->blocklistsize, startofnewblocklist);
			if (0 != res)
				return res;
			wipe_bitlist(dev, 0,
				     startofnewblocklist + dev->blocklistsize,
				     newblocklistsize - dev->blocklistsize);
			if (0 != (res = tier_sync(dev)))
				return res;
			dev->backdev[0]->startofblocklist = startofnewblocklist;
			dev->blocklistsize = newblocklistsize;
			dev->backdev[0]->devmagic->blocklistsize =
			    newblocklistsize;
			dev->backdev[0]->devmagic->startofblocklist =
			    startofnewblocklist;
			dev->backdev[0]->endofdata =
			    dev->backdev[0]->startofblocklist - 1;
			write_device_magic(dev, 0);
		} else
			TIERINFO
			    ("startofnewblocklist %llu, old start %llu, no migration needed",
			     startofnewblocklist,
			     dev->backdev[0]->startofblocklist);
	}
	if (devicenr == 0)
		dev->backdev[devicenr]->endofdata = startofnewblocklist - 1;
	else
		dev->backdev[devicenr]->endofdata = startofnewbitlist - 1;
	dev->backdev[devicenr]->startofbitlist = startofnewbitlist;
	dev->backdev[devicenr]->bitlistsize = newbitlistsize;
	dev->backdev[devicenr]->devmagic->bitlistsize = newbitlistsize;
	dev->backdev[devicenr]->devmagic->startofbitlist = startofnewbitlist;
	dev->backdev[devicenr]->devmagic->devicesize = newdevsize;
	dev->backdev[devicenr]->devicesize = newdevsize;
	write_device_magic(dev, devicenr);
	res = tier_sync(dev);
	return res;
}

static void resize_tier(struct tier_device *dev)
{
	int count;
	int res = 1;
	u64 curdevsize = 0;
	u64 newbitlistsize = 0;
	u64 newblocklistsize = 0;
	u64 newdevsize = 0;
	u64 newbitlistsize_total = 0;
	int found = 0;

	for (count = 0; count < dev->attached_devices; count++) {
		curdevsize =
		    KERNEL_SECTORSIZE *
		    tier_get_size(dev->backdev[count]->tier_dta_file);
		curdevsize = round_to_blksize(curdevsize);
		newbitlistsize = calc_bitlist_size(curdevsize);
		TIERINFO("curdevsize = %llu old = %llu", curdevsize,
			 dev->backdev[count]->devicesize);
		if (dev->backdev[count]->devicesize == curdevsize)
			continue;
		if (curdevsize - dev->backdev[count]->devicesize <
		    newbitlistsize) {
			TIERINFO
			    ("Ignoring unusable small devicesize change for device %u",
			     count);
			continue;
		}
		newdevsize = calc_new_devsize(dev, count, curdevsize);
		newbitlistsize_total =
		    new_total_bitlistsize(dev, count, newbitlistsize);
		newblocklistsize =
		    calc_blocklist_size(newdevsize, newbitlistsize_total);
		// Make sure there is plenty of space
		if (curdevsize <
		    dev->backdev[count]->devicesize + newblocklistsize +
		    newbitlistsize + BLKSIZE) {
			TIERINFO
			    ("Ignoring unusable small devicesize change for device %u",
			     count);
			continue;
		}
		found++;
		TIERINFO("newblocklistsize=%llu", newblocklistsize);
		res =
		    do_resize_tier(dev, count, curdevsize, newblocklistsize,
				   newbitlistsize, curdevsize);
	}
	if (0 == found) {
		TIERINFO
		    ("Ignoring request to resize, no devices have changed in size");
	} else {
		if (res == 0) {
			free_blocklist(dev);
			TIERINFO("Device %s is resized from %llu to %llu",
				 dev->devname, dev->size,
				 newdevsize - newblocklistsize -
				 newbitlistsize_total);
			dev->size =
			    newdevsize - newblocklistsize -
			    newbitlistsize_total;
			dev->backdev[0]->devmagic->total_device_size =
			    dev->size;
			register_new_device_size(dev);
			load_blocklist(dev);
		}
	}
	return;
}

static long tier_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct tier_device *tier = NULL;
	struct tier_device *tiernew = NULL;
	int current_device_nr;
	int err = 0;
	if (!capable(CAP_SYS_ADMIN))
		return -EACCES;

	mutex_lock(&ioctl_mutex);
	if (cmd != TIER_INIT)
		tier = device_nr(&current_device_nr);
	if (NULL == tier && cmd != TIER_INIT) {
		err = -EBADSLT;
		goto end_error;
	}
	switch (cmd) {
	case TIER_INIT:
		if (sizeof(struct tier_device) > TIER_HEADERSIZE)
			return -ENOMEM;
		tiernew = kzalloc(sizeof(struct tier_device), GFP_KERNEL);
		if (NULL == tiernew)
			return -ENOMEM;
		if (0 == tier_device_count()) {
			device = tiernew;
		}
		list_add_tail(&tiernew->list, &device_list);
		tiernew->backdev =
		    kzalloc(sizeof(struct backing_device *) * MAX_BACKING_DEV,
			    GFP_KERNEL);
		if (NULL == tiernew)
			return -ENOMEM;
		break;
	case TIER_SET_FD:
		err = -EEXIST;
		if (tier->attached_devices > MAX_BACKING_DEV)
			break;
		if (0 != tier->tier_device_number)
			break;
		tier->backdev[tier->attached_devices] =
		    kzalloc(sizeof(struct backing_device), GFP_KERNEL);
		err = tier_set_fd(tier, arg, tier->attached_devices);
		tier->attached_devices++;
		break;
	case TIER_SET_SECTORSIZE:
		err = -EEXIST;
		if (0 != tier->tier_device_number)
			break;
		err = 0;
		tier->logical_block_size = arg;
		TIERINFO("sectorsize : %d", tier->logical_block_size);
		break;
	case TIER_REGISTER:
		err = -EEXIST;
		if (0 != tier->tier_device_number)
			break;
		if (0 == tier->attached_devices) {
			TIERERR("Insufficient parameters entered");
		} else {
			tier->tier_device_number = current_device_nr;
			order_devices(tier);
			if (0 == (err = determine_device_size(tier))) {
				err = tier_register(tier);
			}
		}
		break;
	case TIER_DEREGISTER:
		err = tier_device_count();
		del_tier_device((char *)arg);
		if (1 == err)
			device = NULL;
		err = 0;
		break;
	default:
		err = tier->ioctl ? tier->ioctl(tier, cmd, arg) : -EINVAL;
	}
      end_error:
	mutex_unlock(&ioctl_mutex);
	return err;
}

static const struct file_operations _tier_ctl_fops = {
	.open = nonseekable_open,
	.unlocked_ioctl = tier_ioctl,
	.owner = THIS_MODULE,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
	.llseek = noop_llseek
#else
	.llseek = no_llseek
#endif
};

static struct miscdevice _tier_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "tiercontrol",
	.nodename = "tiercontrol",
	.fops = &_tier_ctl_fops
};

static int __init tier_init(void)
{
	int r;
	/* First register out control device */

	TIERINFO("version    : %s", TIER_VERSION);
	r = misc_register(&_tier_misc);
	if (r) {
		TIERERR("misc_register failed for control device");
		return r;
	}
	/*
	 * Alloc our device names
	 */
	init_devicenames();
	mutex_init(&ioctl_mutex);
	return 0;
}

static void __exit tier_exit(void)
{
	struct tier_device *tier, *next;

	list_for_each_entry_safe(tier, next, &device_list, list)
	    tier_deregister(tier);

	if (misc_deregister(&_tier_misc) < 0)
		TIERERR("misc_deregister failed for tier control device");
	kfree(devicenames);
	mutex_destroy(&ioctl_mutex);
}

module_init(tier_init);
module_exit(tier_exit);
