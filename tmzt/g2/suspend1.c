#include <linux/module.h>
#include <linux/init.h>

#include <linux/version.h>

#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
//#include "../drivers/mmc/host/msm_sdcc.h"

#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/genhd.h>

#include <linux/device.h>
#include <linux/platform_device.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32))
#define disk_to_dev(disk)       (&(disk)->part[0]->dev)
#else
#define disk_to_dev(disk)       (&(disk)->part0.__dev)
#endif

static void host_off(struct mmc_host *host) {
    struct device *dev;
    struct device *class_dev;
    struct platform_device *pdev;

    dev = host->parent;
    pdev = to_platform_device(dev);
    printk("finding the platform device: %s\n", pdev->name);
    printk("save this value: pdev: %.8x\n", pdev);
    #ifdef DOSTUFF
    printk("suspending the device: %s\n", pdev->name);
    platform_pm_suspend(dev);
    printk("done.\n");    
    #else
    #warning "nop'd version"
    printk("not suspending device: %s\n", pdev->name);
    #endif
    return;
}

static struct mmc_host *get_host(const char* path) {
    struct block_device *bd = lookup_bdev(path);
    if (IS_ERR(bd)) {
        printk("%s: invalid block device (are you on android/mdev?)\n", path);
        return (struct mmc_host *)-ENODEV;
    }
    printk("%s: block_device: %.8x\n", path, (unsigned int)bd);
    struct gendisk *gd = bd->bd_disk;
    printk("%s: gendisk: %.8x\n", path, (unsigned int)gd);
    #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32))
    struct device *part_dev = disk_to_dev(gd);
    printk("%s: (part) device: %.8x\n", path, (unsigned int)part_dev);
    printk("%s: (part) device name: %s\n", path, dev_name(part_dev));
    struct device *disk_dev = part_dev->parent;
    #else
    struct device *disk_dev = disk_to_dev(gd);
    #endif
    printk("%s: (disk) device: %.8x\n", path, (unsigned int)disk_dev);
    printk("%s: (disk) device name: %s\n", path, dev_name(disk_dev));
    struct device *card_dev = disk_dev->parent;
    printk("%s: (card) device: %.8x\n", path, (unsigned int)card_dev);
    printk("%s: (card) device name: %s\n", path, dev_name(card_dev));
    struct mmc_card *card = container_of(card_dev, struct mmc_card, dev);
    printk("%s: card: %.8x\n", path, (unsigned int)card);
    struct mmc_host *host = card->host;
    printk("%s: host: %.8x\n", path, (unsigned int)host);
    return host;

//  struct mmc_card *card = bdget(MKDEV(179, 0))->bd_disk->part0->__dev.parent;
//  struct mmc_host *host = container_of(card, struct mmc_card, dev)->host;
//  struct mmc_host *host = container_of(card, struct mmc_card, dev)->host;
}

static int test_card(const char* path) {
    struct mmc_host *host = get_host(path);
    if (IS_ERR(host)) return -ENODEV;
    printk("found mmc host: %.8x\n", host);
    printk("found mmc host name: %.8x\n", mmc_hostname(host));
    host_off(host);
    return 0;
}

static int __init test_init(void) {
        printk("test module loaded\n");

        int res = test_card("/dev/mmcblk0");
        if (res < 0) res = test_card("/dev/block/mmcblk0");
    	return 0;
}

static void __exit test_exit(void) {
        printk("test module unloaded\n");
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
