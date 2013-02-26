#ifdef __KERNEL__
#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/file.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/scatterlist.h>
#include <linux/workqueue.h>
#include <linux/rbtree.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/falloc.h>
#include <linux/kthread.h>
#include <linux/version.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/net.h>
#include <linux/inet.h>
#include <asm/div64.h>
#include "btier_log.h"
#else
typedef unsigned long long  u64;
typedef unsigned long u32;
#include <time.h>
#endif


#define BLKSIZE 1048576  /*Moving smaller blocks then 4M around
                           will lead to fragmentation*/
#define BLKBITS 20       /*Adjust when changing BLKSIZE*/
#define TIER_NAME_SIZE     64  /* Max lenght of the filenames*/
#define TIER_SET_FD        0xFE00
#define TIER_SET_DEVSZ     0xFE03
#define TIER_REGISTER      0xFE04
#define TIER_DEREGISTER    0xFE05
#define TIER_INIT          0xFE07
#define TIER_BARRIER       0xFE08
#define TIER_CACHESIZE     0xFE09
#define TIER_SET_SECTORSIZE  0xFE0A
#define TIER_HEADERSIZE    1048576
#define TIER_DEVICE_BIT_MAGIC  0xabe
#define TIER_DEVICE_BLOCK_MAGIC  0xafdf

#define WT 1 /* Write through */
#define WB 2 /* Write back */
#define WD 3 /* Write delayed */

#define RANDOM 0x01
#define SEQUENTIAL 0x02
#define KERNEL_SECTORSIZE 512
#define MAX_BACKING_DEV 16
/* Tier reserves 2 MB per device for playing data migration games. */
#define TIER_DEVICE_PLAYGROUND BLKSIZE*2

#define CLEAN 1
#define DIRTY 2
#define MASTER 0
#define SLAVE 1
#define EST 1
#define DIS 2

#define TIERREAD 1
#define TIERWRITE 2

#define TIERMAXAGE 86400          /* When a chunk has not been used TIERMAXAGE it
                                     will migrate to a slower (higher) tier */
#define TIERHITCOLLECTTIME 43200  /* Every block has TIERHITCOLLECTTIME to collect hits before
                                     being migrated when it has less hits than average */ 
#define MIGRATE_INTERVAL 14400    /* Check every 4 hours */

struct blockinfo {
   unsigned int device;
   u64 offset;
   time_t lastused;
   unsigned int readcount;
   unsigned int writecount;
} __attribute__ ((packed));

struct devicemagic {
   unsigned int magic;
   unsigned int devicenr;
   unsigned int clean;
   u64 blocknr_journal;
   struct blockinfo binfo_journal_new;
   struct blockinfo binfo_journal_old;
   unsigned int average_reads;
   unsigned int average_writes;
   u64 total_reads;
   u64 total_writes;
   time_t average_age;
   u64 devicesize;
   u64 total_device_size;   /* Only valid for tier 0 */
   u64 total_bitlist_size;  /* Only valid for tier 0 */
   u64 bitlistsize;
   u64 blocklistsize;
   u64 startofbitlist;
   u64 startofblocklist;
   char fullpathname[1025];
} __attribute__ ((packed));

#ifdef __KERNEL__

typedef struct {
    struct file *fp;
    mm_segment_t fs;
} file_info_t;

struct data_policy {
       unsigned int max_age;
       unsigned int hit_collecttime;
};

struct backing_device {
        struct file * tier_dta_file;
        u64 bitlistsize;
        u64 devicesize;
        u64 startofdata;
        u64 endofdata;
        u64 startofbitlist;
        u64 startofblocklist;
        u64 bitbufoffset;
        u64 free_offset;
        u64 usedoffset;
        unsigned int shouldsync;
        struct   devicemagic *devmagic;
        struct kobject *ex_kobj;
        struct data_policy dtapolicy;
        char **blocklist;
        char *bitlist;
        unsigned int ra_pages;
};

struct tier_stats {
        u64 seq_reads;          
        u64 rand_reads;          
        u64 seq_writes;        
        u64 rand_writes;        
};

struct tier_device {
        struct list_head list;
        int                     major_num;
        int                     tier_device_number;
        int                     active;
        int                     attached_devices;
        int                     (*ioctl)(struct tier_device *, int cmd, 
                                     u64 arg); 
        u64                     nsectors;
        unsigned int            logical_block_size;
        struct backing_device   **backdev;
        struct block_device     *tier_device;
        struct mutex            tier_ctl_mutex;
        u64                     size;
        u64                     blocklistsize;
        spinlock_t              lock;
        struct gendisk          *gd;
        struct workqueue_struct *migration_queue; /* Migration */
        struct task_struct      *tier_thread;
        struct bio_list         tier_bio_list;
        struct request_queue    *rqueue;
        char                    *devname;
        char                    *managername;
        atomic_t                migrate; 
        atomic_t                wcachelock; 
        atomic_t                commit; 
        unsigned int            commit_interval;
        int                     barrier;
        int                     stop;
/*Holds the type of IO random or sequential*/
        int                     iotype;
/*Last blocknr written or read*/
        u64                     lastblocknr;  
        u64                     resumeblockwalk;
/*Incremented if current blocknr == lastblocknr -1 or +1 */
        unsigned int            insequence;  
        u64                     cacheentries;
        struct mutex            cachelock;
        wait_queue_head_t       tier_event;
        wait_queue_head_t       cache_event;
        wait_queue_head_t       migrate_event;
        struct timer_list       migrate_timer;
        struct tier_stats       stats;
        int                     migration_disabled;
        u64                     migration_interval;
        int                     migrate_verbose;
        int                     ptsync;
/* Where do we initially store sequential IO */
        int                     sequential_landing; 
        int                     inerror;
};

typedef struct {
        struct work_struct work;
        struct tier_device *device;
} tier_worker_t;
#endif
