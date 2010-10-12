#include <linux/module.h>
#include <linux/init.h>

#include <linux/version.h>

#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
//#include "../drivers/mmc/host/msm_sdcc.h"

#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/genhd.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32))
#define disk_to_dev(disk)       (&(disk)->part[0]->dev)
#else
#define disk_to_dev(disk)       (&(disk)->part0.__dev)
#error
#endif

static void host_off(struct mmc_host *host) {
    printk("not turning off: %s\n", mmc_hostname(host));
    return;
}

static struct mmc_host *get_host(const char* path) {
    struct block_device *bd = lookup_bdev(path);
    printk("%s: block_device: %.8x\n", path, (unsigned int)bd);
    struct gendisk *gd = bd->bd_disk;
    printk("%s: gendisk: %.8x\n", path, (unsigned int)gd);
    struct device *dev = disk_to_dev(gd);
    printk("%s: device: %.8x\n", path, (unsigned int)dev);
    struct mmc_card *card = container_of(card, struct mmc_card, dev);
    printk("%s: card: %.8x\n", path, (unsigned int)card);
    struct mmc_host *host = card->host;
    printk("%s: host: %.8x\n", path, (unsigned int)host);
    return host;

//  struct mmc_card *card = bdget(MKDEV(179, 0))->bd_disk->part0->__dev.parent;
//  struct mmc_host *host = container_of(card, struct mmc_card, dev)->host;
//  struct mmc_host *host = container_of(card, struct mmc_card, dev)->host;
}

static void test_card(const char* path) {
    struct mmc_host *host = get_host(path);
    host_off(host);
}

static int __init test_init(void) {
        printk("test module loaded\n");

        test_card("/dev/mmcblk0");

    	return 0;
}

static void __exit test_exit(void) {
        printk("test module unloaded\n");
}

module_init(test_init);
module_exit(test_exit);
