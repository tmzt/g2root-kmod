
#include <linux/module.h>
#include <asm/types.h>
#include <stddef.h>
#include "ssbi2.h"
#include "pmic.h"

static void mpp_test(void) {

    u8 mpp;
    u8 val = 0x0;
    u8 type;
    u8 level;
    u8 control;

/*
	val  = (type << PM8901_MPP_TYPE___S) & PM8901_MPP_TYPE___M;
	val |= (level << PM8901_MPP_CONFIG_LVL___S) & PM8901_MPP_CONFIG_LVL___M;
	val |= (control << PM8901_MPP_CONFIG_CTL___S) & PM8901_MPP_CONFIG_CTL___M;

	return pm8901_write(&val, 1, PM8901_MPP_CONTROL(mpp));
*/

    for (mpp = 0; mpp < 4; mpp++) {
        pm8901_read(&val, 1, PM8901_MPP_CONTROL(mpp));
        type = (val & PM8901_MPP_TYPE___M) >> PM8901_MPP_TYPE___S;
        level = (val & PM8901_MPP_CONFIG_LVL___M) >> PM8901_MPP_CONFIG_LVL___S;
        control = (val & PM8901_MPP_CONFIG_CTL___M) >> PM8901_MPP_CONFIG_CTL___S;
        printk("mpp: %d\n", mpp);
        printk("val: %u\n", val);
        printk("type: %u\n", type);
        printk("control: %u\n", control);
    };
}

static int __init test_init(void) {
    printk("test module loaded\n");
    mpp_test();
	return 0;
}

static void __exit test_exit(void) {
    printk("test module unloaded\n");
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
