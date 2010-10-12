#include <linux/module.h>
#include <linux/init.h>

#include <linux/fs.h>
#include <linux/genhd.h>

static void show_ops(const char *path) {
    struct block_device *bd;

    bd = lookup_bdev(path);
    printk("%s: block_device: %.8x\n", path, (unsigned int)bd);
}

static void show_all() {
    show_ops("/dev/mmcblk0");
}

static int __init test_init(void) {
        printk("test module loaded\n");
        show_all();
	return 0;
}

static void __exit test_exit(void) {
        printk("test module unloaded\n");
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
