#define dmesg printk("wpthis: "); printk

#define get_or_die(target, source)					               \
    target = source;							               \
    if(!target)								               \
    {									               \
	dmesg("%s = %s == null.\n", #source, #target);                                 \
	goto out;                                                                      \
    }									               \
    else if(IS_ERR(target))						               \
    {									               \
	dmesg("%s gave us an error for %s: %lu\n", #source, #target, PTR_ERR(target)); \
	goto out;                                                                      \
    }





#define print_dev(device) \
    dmesg("%s: 0x%.8x (%s)\n", #device, (unsigned int)device, dev_name(device))

#define print_clock(clock) \
    dmesg("%s: 0x%.8x (%lu)\n", #clock, (unsigned int)clock, clk_get_rate(clock));

#define assert(eval)                               \
    if(!(eval))                                    \
    {                                              \
	dmesg("assertion failed! !(%s)\n", #eval); \
	goto out;                                  \
    }

