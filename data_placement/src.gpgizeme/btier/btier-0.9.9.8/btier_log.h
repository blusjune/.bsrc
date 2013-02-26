#define TIERCRIT(f, arg...) \
        printk(KERN_CRIT "TIER : " f "\n", ## arg)
#define TIERERR(f, arg...) \
        printk(KERN_NOTICE "TIER : " f "\n", ## arg)
#define TIERINFO(f, arg...) \
        printk(KERN_INFO "TIER : " f "\n", ## arg)
#ifdef DEBUG
#define TIERDEBUG(f, arg...) \
        if ( debug ) printk(KERN_DEBUG "TIER : " f "\n", ## arg)
#else
#define TIERDEBUG(f, arg...)  
#endif
