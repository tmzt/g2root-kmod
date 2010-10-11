#include <linux/module.h>
#include <linux/init.h>

static int __init test_init(void) {
        printk("test module loaded\n");
	return 0;
}

static void __exit test_exit(void) {
        printk("test module unloaded\n");
}

module_init(test_init);
module_exit(test_exit);
