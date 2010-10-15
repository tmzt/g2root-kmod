#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>

#include <mach/vreg.h>

#include "../arch/arm/mach-msm/proc_comm.h"

int vreg_id = 0;

int (*stupid_proc_comm)(uint, uint*, uint*) = (void *)0xc003bdbc;

static int __init test_init(void) {
    int res;
    int id;
    int enable;

    printk("test module loaded\n");

    id = vreg_id;
    if (!id) {
        printk("bad vreg_id parameter: %d\n", vreg_id);
        return -EINVAL;
    }    

    printk("turning off GP12 (#%d)\n", id);
    id = 37;
    enable = 0;
    res = stupid_proc_comm(PCOM_VREG_SWITCH, &id, &enable);
    printk("result: %d\n", res);
    printk("done.\n");
	return 0;
}

static void __exit test_exit(void) {
        printk("test module unloaded\n");
}

module_init(test_init);
module_exit(test_exit);

module_param(vreg_id, int, 0444);

MODULE_LICENSE("GPL");
