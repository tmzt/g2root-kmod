#include <linux/module.h>
#include <linux/init.h>
#include <linux/unistd.h>
#include <linux/delay.h>

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/earlysuspend.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/gpio.h>

#include <linux/version.h>

#include <linux/mmc/host.h>
#include <linux/mmc/card.h>

#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/genhd.h>

#include <linux/device.h>
#include <linux/platform_device.h>

#include <mach/irqs.h>
#include <mach/msm_iomap.h>
#include <mach/dma.h>
#include <mach/board.h>

#include "../arch/arm/mach-msm/smd_private.h"
#include "../arch/arm/mach-msm/devices.h"
#include "../arch/arm/mach-msm/clock.h"
#include "../arch/arm/mach-msm/proc_comm.h"
#include <mach/gpio.h>
#include <mach/msm_hsusb.h>
#include <mach/msm_rpcrouter.h>
#include <mach/msm_hsusb_hw.h>

#include "../drivers/mmc/host/msm_sdcc.h"

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32))
#define disk_to_dev(disk)       (&(disk)->part[0]->dev)
#else
#define disk_to_dev(disk)       (&(disk)->part0.__dev)
#endif

static uint32_t emmc_wp_normal_table[] = {
    PCOM_GPIO_CFG(88, 2, GPIO_INPUT, GPIO_NO_PULL, GPIO_4MA),
};

static uint32_t emmc_wp_reset_table[] = {
    PCOM_GPIO_CFG(88, 2, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_4MA),
};

static struct resource resources_sdc2[] = {
	{
		.start	= MSM_SDC2_PHYS,
		.end	= MSM_SDC2_PHYS + MSM_SDC2_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= INT_SDC2_0,
		.end	= INT_SDC2_0,
		.flags	= IORESOURCE_IRQ,
		.name	= "cmd_irq",
	},
		{
		.start	= INT_SDC2_1,
		.end	= INT_SDC2_1,
		.flags	= IORESOURCE_IRQ,
		.name	= "pio_irq",
	},
	{
		.flags	= IORESOURCE_IRQ | IORESOURCE_DISABLED,
		.name	= "status_irq"
	},
	{
		.start	= 8,
		.end	= 8,
		.flags	= IORESOURCE_DMA,
	},
};

struct platform_device msm_device_sdc2 = {
	.name		= "msm_sdcc",
	.id		= 2,
	.num_resources	= ARRAY_SIZE(resources_sdc2),
	.resource	= resources_sdc2,
	.dev		= {
		.coherent_dma_mask	= 0xffffffff,
	},
};

static struct platform_device *get_pdev(struct mmc_host *host) {
    struct device *dev;
    struct platform_device *pdev;

    if (!host->parent) return (void*)-ENODEV;
    dev = host->parent;
    pdev = to_platform_device(dev);
    return pdev;
}

static int rereg(void) {
    platform_device_register(&msm_device_sdc2);
    printk("done.\n");
    return 0;
}    

static int host_off(struct mmc_host *host) {
    struct platform_device *pdev;

    printk("host_off: removing mmc_host: %s\n", mmc_hostname(host));
    mmc_remove_host(host);

    pdev = get_pdev(host);
    printk("host_off: pdev: %.8x\n", (unsigned int)pdev);
    if (!pdev) return -ENODEV;

    printk("unregistering the platform device: %s", pdev->name);
    platform_device_unregister(pdev);
    
    return 0;
}

static struct mmc_host *get_host(const char* path) {
    struct block_device *bd = lookup_bdev(path);
    struct gendisk *gd;
    #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32))    
    struct device *part_dev;
    #endif
    struct device *disk_dev;
    struct device *card_dev;
    struct mmc_card *card;
    struct mmc_host *host;

    if (IS_ERR(bd)) {
        printk("%s: invalid block device (are you on android/mdev?)\n", path);
        return (struct mmc_host *)-ENODEV;
    }
    printk("%s: block_device: %.8x\n", path, (unsigned int)bd);
    gd = bd->bd_disk;
    printk("%s: gendisk: %.8x\n", path, (unsigned int)gd);
    #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32))
    part_dev = disk_to_dev(gd);
    printk("%s: (part) device: %.8x\n", path, (unsigned int)part_dev);
    printk("%s: (part) device name: %s\n", path, dev_name(part_dev));
    disk_dev = part_dev->parent;
    #else
    disk_dev = disk_to_dev(gd);
    #endif
    printk("%s: (disk) device: %.8x\n", path, (unsigned int)disk_dev);
    printk("%s: (disk) device name: %s\n", path, dev_name(disk_dev));
    card_dev = disk_dev->parent;
    printk("%s: (card) device: %.8x\n", path, (unsigned int)card_dev);
    printk("%s: (card) device name: %s\n", path, dev_name(card_dev));
    card = container_of(card_dev, struct mmc_card, dev);
    printk("%s: card: %.8x\n", path, (unsigned int)card);
    host = card->host;
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
    printk("removing mmc_host");
    host_off(host);

    printk("pulling down gpio 88\n");
    config_gpio_table(emmc_wp_reset_table, ARRAY_SIZE(emmc_wp_reset_table));
    mdelay(1000);
    printk("returning gpio 88 to normal\n");
    config_gpio_table(emmc_wp_normal_table, ARRAY_SIZE(emmc_wp_normal_table));
    printk("done.\n");

    printk("restoring in 5 seconds\n");
    msleep(5);
    rereg();
    printk("done.\n");
    return 0;
}

static int __init test_init(void) {
        int res = 0;
        printk("test module loaded\n");
        res = test_card("/dev/mmcblk0");
        if (res < 0) res = test_card("/dev/block/mmcblk0");
    	return res;
}

static void __exit test_exit(void) {
        printk("test module unloaded\n");
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
