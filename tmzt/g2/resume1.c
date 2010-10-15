#include <linux/module.h>
#include <linux/init.h>

#include <linux/version.h>

#include <linux/platform_device.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
#include "../arch/arm/mach-msm/include/mach/irqs.h"
#endif

#include "../arch/arm/mach-msm/include/mach/msm_iomap.h"

unsigned int pdev_addr;

static int resume(struct platform_device *pdev) {
    struct device *dev = NULL;

    bool namevalid = false;

    printk("using passed pdev value: %.8x\n", (unsigned int)pdev);
    printk("sanity checking, is name NULL?\n");
    printk("(pdev->name): %.8x\n", (unsigned int)(pdev->name));
    if (pdev->name) {
        printk("no. continuing\n");
        printk("pdev->name: %s\n", pdev->name);
        namevalid = (strcmp(pdev->name, "msm_sdcc") == 0);
        printk("sanity checking, is name == \"msm_sdcc\"?: %s\n", namevalid ? "yes":"no");
        if (namevalid) {
            printk("yes. continuing\n");
            printk("getting dev.\n");
            dev = &(pdev->dev);
            printk("dev: %.8x\n", (unsigned int)dev);
            if (dev != NULL) {
                printk("not NULL, continuing.\n");
                printk("probing...\n");
//                platform_drv_probe(dev);
            } else { return -3; }
        } else { return -2; }
    } else { return -1; }
    return 0;
}


static int __init test_init(void) {
    struct platform_device *pdev = NULL;

    printk("test module loaded\n");

    printk("pdev_addr: %.8x (will be checked)\n", pdev_addr);
    if (pdev_addr <= 0) {
        printk("bad address. failing.\n");
        return -ENODEV;
    }
    pdev = (struct platform_device *)pdev_addr;
    resume(pdev);
	return 0;
}

static void __exit test_exit(void) {
        printk("test module unloaded\n");
}

module_param_named(pdev_addr, pdev_addr, uint, 0644);

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
