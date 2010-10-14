#include <linux/module.h>
#include <linux/init.h>
#include <linux/unistd.h>
#include <linux/delay.h>
#include <linux/pagemap.h>
#include <linux/dma-mapping.h>
#include <linux/ioport.h>
#include <linux/clk.h>

#include <linux/version.h>

#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
//#include "../drivers/mmc/host/msm_sdcc.h"

#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/genhd.h>

#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/err.h>

#include <mach/msm_iomap.h>
#include <mach/dma.h>
#include <mach/htc_pwrsink.h>

#include "../drivers/mmc/host/msm_sdcc.h"

#include "../scotty2/wp_core.h"
#include "../scotty2/wp_sdcc.h"

#define MSM_SDCC_FMIN 144000L

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32))
#define disk_to_dev(disk)       (&(disk)->part[0]->dev)
#else
#define disk_to_dev(disk)       (&(disk)->part0.__dev)
#endif

void sdcc_writel(u32 data, unsigned int address, struct clk *clock);

void *pdev_page = NULL;

static struct device *get_dev(struct mmc_host *host) {
    struct device *dev;
    if (!host->parent) return (void*)-ENODEV;
    dev = host->parent;
    return dev;
}

static struct platform_device *get_pdev(struct mmc_host *host) {
    struct platform_device *pdev;
    pdev = to_platform_device(get_dev(host));
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
                printk("checking kobj.state_initialized: %s", (pdev->dev.kobj.state_initialized) ? "true":"false");
                if (pdev->dev.kobj.state_initialized) {
                    printk("clearing.\n");
                    pdev->dev.kobj.state_initialized = 0;
                }
                platform_device_register(pdev);
                printk("done.\n");
            } else return -3;
        } else return -2;
    } else return -1;
    return 0;
}    

static int scotty_it(struct platform_device *pdev) {
    uint status;
    int retval;

    uint pwr = 0;
    uint clk = 0;
    void *virt;
    struct clk *host_clk;
    struct clk *host_pclk;

    host_clk = clk_get(&(pdev->dev), "sdc_clk");
    host_pclk = clk_get(&(pdev->dev), "sdc_pclk");
    if (IS_ERR(host_clk)) {
        printk("scotty_it: failed to get sdc_clk: %lu\n", PTR_ERR(host_clk));
        return -ENODEV;
    }

    if (IS_ERR(host_pclk)) {
        printk("scotty_it: failed to get sdc_pclk: %lu\n", PTR_ERR(host_pclk));
        return -ENODEV;
    }

    if (clk_enable(host_clk)) {
        printk("scotty_it: failed to enabled sdc_clk.\n");
        return -ENODEV;
    }

    if (clk_enable(host_pclk)) {
        printk("scotty_it: failed to enabled sdc_pclk.\n");
        return -ENODEV;
    }

    printk("scotty_it: attempting to map controller\n");
    virt = ioremap(MSM_SDC2_PHYS, PAGE_SIZE);
    printk("scotty_it: virtual address: 0x%.8x\n", (uint)virt);
    
    clk = readl((uint)virt + MMCICLOCK);
    printk("scotty_it: MMCICLOCK: 0x%.8x\n", clk);

    pwr = readl((uint)virt + MMCIPOWER);
    printk("scotty_it: MMCIPOWER reg: 0x%.8x\n", pwr);

    pwr = MCI_PWR_OFF;

    printk("scotty_it: MMCIPOWER (powered down): 0x%.8x\n", pwr);
    printk("scotty_it: powering down sdc2...\n");

    sdcc_writel(pwr, (uint)virt + MMCIPOWER, host_clk);

    mmc_delay(1000);

    printk("scotty_it: powering up...\n");

    pwr = MCI_PWR_UP;
    sdcc_writel(pwr, (uint)virt + MMCIPOWER, host_clk);

    mmc_delay(10);

    pwr = readl((uint)virt + MMCIPOWER);
    printk("scotty_it: MMCIPOWER reg: 0x%.8x\n", pwr);

    mmc_delay(10);

    printk("scotty_it: reenabling clock...\n");

    if (clk_enable(host_clk)) {
        printk("scotty_it: failed to enabled sdc_clk.\n");
        return -ENODEV;
    }

    printk("scotty_it: setting clock to %lu...\n", MSM_SDCC_FMIN);
    retval = clk_set_rate(host_clk, MSM_SDCC_FMIN);
    printk("scotty_it: retval: %d\n", retval);

    clk = 0;
    clk |= MCI_CLK_ENABLE;
    clk |= (1 << 12);
    clk |= (1 << 15);
    sdcc_writel(clk, (uint)virt + MMCICLOCK, host_clk);

    mmc_delay(10);

    printk("scotty_it: powering on...\n");
    pwr = MCI_PWR_ON;
    sdcc_writel(pwr, (uint)virt + MMCIPOWER, host_clk);

    mmc_delay(10);

    pwr = readl((uint)virt + MMCIPOWER);
    printk("scotty_it: MMCIPOWER reg: 0x%.8x\n", pwr);

    printk("scotty_it: unmapping controller...\n");
    iounmap(virt);
    printk("scotty_it: MSM_SDCC2_BASE unmapped.\n");

    /* no commands */
    return 0;
}

void sdcc_writel(u32 data, unsigned int address, struct clk *clock)
{
    writel(data, address);
    /* 3 clk delay required! */
    udelay(1 + ((3 * USEC_PER_SEC) /
	    (clk_get_rate(clock) ? clk_get_rate(clock) : MSM_SDCC_FMIN)));
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
    printk("saving platform device\n");
    save(pdev);
    printk("removing host and platform device\n");
    host_off(host);
    printk("restoring in 5 seconds");
    msleep(5);
    printk("resetting controller/card");
    scotty_it(pdev);
    printk("reregistering platform device\n");
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
