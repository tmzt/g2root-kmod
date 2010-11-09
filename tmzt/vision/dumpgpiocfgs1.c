#include <linux/module.h>
#include <linux/init.h>
#include <linux/debugfs.h>
#include <linux/err.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

#include "../arch/arm/mach-msm/gpiocfgs.h"

#include <mach/msm_rpcrouter.h>


static ssize_t gpiocfgs1_debug_read(struct file *file, char __user *buf, ssize_t count, loff_t *ppos) {
    memcpy(buf, gpiocfg1_virt, 1024);
    return 1024;
}

static ssize_t gpiocfgs2_debug_read(struct file *file, char __user *buf, ssize_t count, loff_t *ppos) {
    memcpy(buf, gpiocfg2_virt, 1024);
    return 1024;
}    

static const struct file_operations gpiocfgs_debug_ops = {
        .read = gpiocfgs_debug_read,
};

static const struct file_operations gpiocfgs_debug_ops = {
        .read = gpiocfgs_debug_read,
};

static int __init gpiocfgs_debugfs_init(void) {
    struct dentry *dent;

    dent = debugfs_create_dir("gpiocfgs", 0);
    if (IS_ERR(dent))
        return PTR_ERR(dent);

    debugfs_create_file("gpiocfg1", 0444, dent, NULL, &gpiocfg1_debug_ops);
    debugfs_create_file("gpiocfg2", 0444, dent, NULL, &gpiocfg2_debug_ops);

    return 0;
}

static void __exit gpiocfgs_debugfs_exit(void) {
    printk("gpiocfgs_debugfs_unloaded\n");
}

module_init(gpiocfgs_debugfs_init);
module_exit(gpiocfgs_debugfs_exit);

MODULE_LICENSE("GPL")
