#include <linux/module.h>
#include <linux/init.h>

#include <linux/mmc/host.h>
//#include "../drivers/mmc/host/msm_sdcc.h"

#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/genhd.h>

static void host_off(struct mmc_host *host) {
    return;
}

static void get_host() {
      struct mmc_card *card = bdget(MKDEV(179, 0))->bd_disk->part0->__dev.parent;
      struct mmc_host *host = container_of(card, struct mmc_card, dev)->host;
//    struct mmc_host *host = container_of(card, struct mmc_card, dev)->host;
    host_off(host);
}

static int __init test_init(void) {
        printk("test module loaded\n");
    	return 0;
}

static void __exit test_exit(void) {
        printk("test module unloaded\n");
}

module_init(test_init);
module_exit(test_exit);
