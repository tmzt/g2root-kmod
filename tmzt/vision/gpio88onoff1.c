#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>

#include <linux/delay.h>
#include <linux/gpio.h>

#include <mach/gpio.h>

#include "../arch/arm/mach-msm/proc_comm.h"

static int __init test_init(void) {

    printk("test module loaded\n");
    printk("turning on gpio 88\n");
    gpio_set_value(88, 1);
    mdelay(100);
    printk("turning off gpio 88\n");
    gpio_set_value(88, 0);
    printk("done.\n");
	return 0;
}

static void __exit test_exit(void) {
        printk("test module unloaded\n");
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
