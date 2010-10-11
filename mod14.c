#include <linux/module.h>
#include <linux/init.h>

#include <linux/kobject.h>
#include <linux/platform_device.h>
#include "../fs/sysfs/sysfs.h"

struct kobject *pbus_kobject;
struct sysfs_dirent *sd;
struct sysfs_dirent *dir;

static struct device *get_dev(struct sysfs_dirent *dir);
static int walk_dir(struct sysfs_dirent *dir, char *name, const int linkdepth, void (*found_it)(struct sysfs_dirent *));
static void found_msmsdcc(struct sysfs_dirent *dir);
static void found_mmchost(struct sysfs_dirent *dir);

static struct device *get_dev(struct sysfs_dirent *dir) {
	struct kobject *kobj;
	struct device *dev;

	kobj = dir->s_dir.kobj;	
	printk("get_dev: kobject: %.8x\n", (unsigned int)kobj);
	dev = container_of(kobj, struct device, kobj);
	printk("get_dev: dev: %.8x\n", (unsigned int)dev);
	return dev;
}

static void found_msmsdcc(struct sysfs_dirent *dir) {
	struct platform_device *pdev = container_of(get_dev(dir), struct platform_device, dev);
	printk("found_msmsdcc: pdev: %.8x\n", (unsigned int)pdev);
	printk("pdev name: %s\n", pdev->name);
	//platform_device_unregister(pdev);

	walk_dir(dir, "mmc0", 1, &found_mmchost);
	return;
}

static void found_mmchost(struct sysfs_dirent *dir) {
	struct device *dev = get_dev(dir);
	printk("found_mmchost: dev: %.8x\n", (unsigned int)dev);
}

static int walk_dir(struct sysfs_dirent *dir, char *name, const int linkdepth, void (*found_it)(struct sysfs_dirent *)) {

	struct sysfs_dirent *cur;
	
    printk("walk_dir: name: %s\n", name);
//    printk("walk_dir: linkdepth: %d\n", linkdepth);

//	printk("dirent flags: %d\n", dir->s_flags);
//	printk("dirent name: %s\n", dir->s_name);

	if (strcmp(dir->s_name, name) == 0) {
		printk("found %s: %s\n", name, dir->s_name);
		(*found_it)(dir);
		return 1;
	}

	if (dir->s_flags & SYSFS_DIR) {
		for (cur = dir->s_dir.children; cur->s_sibling != NULL; cur = cur->s_sibling)
		{
		    int retval;

		    printk("name: %s entering directory: %s\n", name, cur->s_name);
		    retval = walk_dir(cur, name, linkdepth, found_it);

		    if(retval)
    			return 1;
		}
		return 0;
	};

	if (dir->s_flags & SYSFS_KOBJ_ATTR) {
		return 0;
	};

	if (dir->s_flags & SYSFS_KOBJ_LINK) {
		printk("name: %s linkdepth: %d\n", name, linkdepth);
		if (linkdepth > 0) {
			printk("following symlink: %s\n", dir->s_symlink.target_sd->s_name);
			walk_dir(dir->s_symlink.target_sd, name, linkdepth-1, found_it);
		};
		return 0;
	};

	return 0;
}

static int __init test_init(void) {
    printk("test module loaded\n");

	pbus_kobject = &platform_bus.kobj;
	sd = pbus_kobject->sd;

	printk("platform_bus kobject: %.8x\n", (unsigned int)pbus_kobject);

	walk_dir(sd, "msm_sdcc.2", 1, &found_msmsdcc);

	return 0;
}

static void __exit test_exit(void) {
        printk("test module unloaded\n");
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
