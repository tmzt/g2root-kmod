#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>

#include <mach/vreg.h>

#include "../arch/arm/mach-msm/proc_comm.h"

static int __init test_init(void) {
    int res;
    int id;
    int enable;

    printk("test module loaded\n");
    printk("turning off GP12 (#37)\n");
    id = 37;
    enable = 0;
    res = msm_proc_comm(PCOM_VREG_SWITCH, &id, &enable);
    printk("result: %d\n", res);
    printk("done.\n");
	return 0;
}

static void __exit test_exit(void) {
        printk("test module unloaded\n");
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
