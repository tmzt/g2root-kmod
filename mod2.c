#include <linux/module.h>
#include <linux/init.h>

#include <linux/platform_device.h>

static int __init test_init(void) {
        printk("test module loaded\n");
	printk("platform_bus: %.8x\n", platform_bus);
	return 0;
}

static void __exit test_exit(void) {
        printk("test module unloaded\n");
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
