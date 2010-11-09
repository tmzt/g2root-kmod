#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>

#include <linux/delay.h>
#include <linux/gpio.h>

#include <mach/gpio.h>

#include "../arch/arm/mach-msm/proc_comm.h"

static uint32_t emmc_wp_normal_table[] = {
    PCOM_GPIO_CFG(88, 2, GPIO_INPUT, GPIO_NO_PULL, GPIO_4MA),
};

static uint32_t emmc_wp_reset_table[] = {
    PCOM_GPIO_CFG(88, 2, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_4MA),
};

static int __init test_init(void) {

    printk("test module loaded\n");
    printk("pulling down gpio 88\n");
    config_gpio_table(emmc_wp_reset_table, ARRAY_SIZE(emmc_wp_reset_table));
//    gpio_set_value(88, 1);
    mdelay(10);
    mdelay(100);
//    mdelay(1000);
    printk("returning gpio 88 to normal\n");
    config_gpio_table(emmc_wp_normal_table, ARRAY_SIZE(emmc_wp_normal_table));
//    gpio_set_value(88, 0);
    printk("done.\n");
	return 0;
}

static void __exit test_exit(void) {
        printk("test module unloaded\n");
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
