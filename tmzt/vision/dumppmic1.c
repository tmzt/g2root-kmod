#include <linux/module.h>
#include <linux/init.h>
#include <linux/debugfs.h>
#include <linux/err.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

#include "../arch/arm/mach-msm/pmic.h"

#include <mach/msm_rpcrouter.h>

static int pmic_rpc_get_struct(uint *data, uint size, int proc) {
    struct {
        struct rpc_request_hdr hdr;
        uint32_t data[32+2];
    } msg;
    struct pmic_reply rep;
    int n = 0;

    msg.data[n++] = cpu_to_be32(
        

static ssize_t smi_debug_read(struct file *file, char __user *buf, ssize_t count, loff_t *ppos) {
    #define MAXPROC 256
    int i;

    if (count >= 4096) return -EINVAL;

    for (i = 0; i < MAXPROC; i++) {

    


static const struct file_operations pmic_debug_ops = {
        .read = pmic_debug_read,
};

static int __init pmic_debugfs_init(void) {
    struct dentry *dent;

    dent = debugfs_create_dir("pmic", 0);
    if (IS_ERR(dent))
        return PTR_ERR(dent);

    debugfs_create_file("smem", 0444, dent, NULL, &pmic_debug_ops);
    return 0;
}

static void __exit pmic_debugfs_exit(void) {
    printk("pmic_debugfs_unloaded\n");
}

module_init(pmic_debugfs_init);
module_exit(pmic_debugfs_exit);

MODULE_LICENSE("GPL")
