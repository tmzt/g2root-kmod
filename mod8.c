#include <linux/module.h>
#include <linux/init.h>

#include <linux/kobject.h>
#include <linux/platform_device.h>
#include "../fs/sysfs/sysfs.h"

struct kobject *pbus_kobject;
struct sysfs_dirent *sd;
struct sysfs_dirent *dir;

static void walk_dir(struct sysfs_dirent *dir) {

	struct sysfs_dirent *cur;
	
//	printk("dirent flags: %d\n", dir->s_flags);
//	printk("dirent name: %s\n", dir->s_name);

	if (strcmp(dir->s_name, "msm_sdcc.2") == 0) {
		printk("found msm_sdcc.2: %s\n", dir->s_name);

		/* get kobject */
		return;
	}

	if (dir->s_flags & SYSFS_DIR) {
		for (cur = dir->s_dir.children; cur->s_sibling != NULL; cur = cur->s_sibling) walk_dir(cur);
		return;
	};

	if (dir->s_flags & SYSFS_KOBJ_ATTR) {
		return;
	};

	return;
}

static int __init test_init(void) {
        printk("test module loaded\n");

	pbus_kobject = &platform_bus.kobj;
	sd = pbus_kobject->sd;

	printk("platform_bus kobject: %.8x\n", (unsigned int)pbus_kobject);

	walk_dir(sd);

	return 0;
}

static void __exit test_exit(void) {
        printk("test module unloaded\n");
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
