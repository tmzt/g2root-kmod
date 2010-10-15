#include <linux/module.h>
#include <linux/init.h>

#include <linux/kobject.h>
#include <linux/platform_device.h>

struct kobject *pbus_kobject;

static int __init test_init(void) {
        printk("test module loaded\n");
	//printk("platform_bus: %.8x\n", (unsigned long)platform_bus);
//	printk("sysfs_root: %.8x\n", (unsigned long)sysfs_root);

	pbus_kobject = &platform_bus.kobj;

	printk("platform_bus kobject: %.8x\n", (unsigned int)pbus_kobject);

	return 0;
}

static void __exit test_exit(void) {
        printk("test module unloaded\n");
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
