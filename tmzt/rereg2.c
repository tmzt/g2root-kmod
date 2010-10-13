#include <linux/module.h>
#include <linux/init.h>

#include <linux/version.h>

#include <linux/platform_device.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
#include "../arch/arm/mach-msm/include/mach/irqs.h"
#endif

#include "../arch/arm/mach-msm/include/mach/msm_iomap.h"

static void rereg() {
    printk("registering platform device...\n");
    platform_device_register(&msm_device_sdc2);
    printk("done\n");
}


static int __init test_init(void) {
        printk("test module loaded\n");
        rereg();
	return 0;
}

static void __exit test_exit(void) {
        printk("test module unloaded\n");
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
