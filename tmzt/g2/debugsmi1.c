#include <linux/module.h>
#include <linux/init.h>
#include <linux/debugfs.h>

#include <linux/pagemap.h>
#include <linux/unistd.h>

#include <asm/io.h>

static ssize_t smi_debug_read(struct file *file, char __user *buf, ssize_t count, loff_t *ppos) {
    int pagecount = 0;
    int pageremain = 0;
    uint smi_off = 0;
    uint smi_page = 0;
    uint off = 0;
    uint pageoff = 0;
    void *smi = NULL;

    smi_off = *ppos;
    smi_page = smi_off & ~(PAGE_SIZE-1);

    while (off < count) {
        pageoff = off & (PAGE_SIZE-1); /* offset in current page */
        smi = ioremap(0x00100000+smi_page, PAGE_SIZE);
        pageremain = (count-off) & (PAGE_SIZE-1);
        pagecount = PAGE_SIZE - pageremain;
        memcpy(buf+off, smi+pageoff, pagecount);
        off += pagecount;
        iounmap(smi);
    };
    *ppos += off;
    return off;
} 

static const struct file_operations smi_debug_ops = {
        .read = smi_debug_read,
};

static int __init smi_debugfs_init(void) {
    struct dentry *dent;

    dent = debugfs_create_dir("smi", 0);
    if (IS_ERR(dent))
        return PTR_ERR(dent);

    debugfs_create_file("smem", 0444, dent, NULL, &smi_debug_ops);
    return 0;
}

static void __exit smi_debugfs_exit(void) {
    printk("smi_debugfs_unloaded\n");
}

module_init(smi_debugfs_init);
module_exit(smi_debugfs_exit);

MODULE_LICENSE("GPL");
