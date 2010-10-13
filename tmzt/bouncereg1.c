#include <linux/module.h>
#include <linux/init.h>
#include <linux/unistd.h>
#include <linux/delay.h>

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

void *pdev_page = NULL;

static struct platform_device *get_pdev(struct mmc_host *host) {
    struct device *dev;
    struct device *class_dev;
    struct platform_device *pdev;

    if (!host->parent) return (void*)-ENODEV;
    dev = host->parent;
    pdev = to_platform_device(dev);
    return pdev;
}

static void save(struct platform_device *pdev) {
    pdev_page = kzalloc(PAGE_SIZE, GFP_KERNEL);
    memcpy(pdev_page, pdev, PAGE_SIZE);
}

static int rereg() {
    struct platform_device *pdev = (struct platform_device *)pdev_page;

    printk("sanity checking our saved pdev\n");
    printk("pdev_page: %.8x\n", (uint)pdev_page);
    printk("checking if pdev_page is NULL\n");
    if (pdev_page) {
        printk("no. continuing.\n");
        printk("checking if the first entry (pdev->name) is NULL\n");
        printk("pdev->name: %.8x\n", (uint)pdev->name);
        if (pdev->name) {
            printk("no.\n");
            printk("pdev->name: %s\n", pdev->name);
            printk("pdev->id: %d\n", pdev->id);
            printk("checking if pdev->id == 2 (sdc2)\n");
            if (pdev->id == 2) {
                printk("yes.\n");
                printk("registering platform device: %s\n", pdev->name);
                platform_device_register(pdev);
                printk("done.\n");
            } else return -3;
        } else return -2;
    } else return -1;
    return 0;
}    

static int host_off(struct mmc_host *host) {
    struct platform_device *pdev;

    #ifdef DOSTUFF

    printk("host_off: removing mmc_host: %s\n", mmc_hostname(host));
    mmc_remove_host(host);

    pdev = get_pdev(host);
    printk("host_off: pdev: %.8x\n", (unsigned int)pdev);
    if (!pdev) return -ENODEV;

    printk("unregistering the platform device: %s", pdev->name);
    platform_device_unregister(pdev);
    
    #else
    #warning "nop'd version"
    printk("not turning off: %s\n", mmc_hostname(host));
    #endif
    return 0;
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
    struct platform_device *pdev;

    host = get_host(path);

    if (!host || IS_ERR(host)) return -ENODEV;
    pdev = get_pdev(host);
    if (!pdev || IS_ERR(pdev)) return -ENODEV;
    save(pdev);
    host_off(host);
    printk("restoring in 5 seconds");
    msleep(5);
    rereg();
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
