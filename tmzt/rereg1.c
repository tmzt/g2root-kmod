#include <linux/module.h>
#include <linux/init.h>

#include <linux/version.h>

#include <linux/platform_device.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
#include "../arch/arm/mach-msm/include/mach/irqs.h"
#endif

#include "../arch/arm/mach-msm/include/mach/msm_iomap.h"

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
        .name           = "msm_sdcc",
        .id             = 2,
        .num_resources  = ARRAY_SIZE(resources_sdc2),
        .resource       = resources_sdc2,
        .dev            = {
                .coherent_dma_mask      = 0xffffffff,
        },
};

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
