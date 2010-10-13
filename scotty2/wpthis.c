#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/pagemap.h>
#include <linux/err.h>
#include <linux/leds.h>
#include <linux/scatterlist.h>
#include <linux/log2.h>
#include <linux/regulator/consumer.h>
#include <linux/wakelock.h>
#include <linux/genhd.h>
#include <linux/kdev_t.h>
#include <linux/mount.h>
#include <linux/platform_device.h>
#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sd.h>
#include <linux/dma-mapping.h>
#include <linux/ioport.h>

#include <asm/io.h>
#include <asm/cacheflush.h>
#include <asm/div64.h>
#include <asm/sizes.h>
#include <asm/mach/mmc.h>

#include <mach/msm_iomap.h>
#include <mach/dma.h>
#include <mach/htc_pwrsink.h>

#include "wp_core.h"
#include "wp_sdcc.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Scott Walker <walker.scott@gmail.com>");
MODULE_DESCRIPTION("i'd like this to disable wp on the emmc chip on my g2, please?");

#define dmesg printk

#define dev_to_mmc_card(d) container_of(d, struct mmc_card, dev)

#define RCA 1
#define KSTART 0xc0008000
#define MSM_SDCC2_BASE 0xa0500000

void have_fun(struct mmc_host *, struct mmc_card *);
unsigned int offset = 0;
u32 getstatus(struct mmc_host *);

// command functions
int mmc_send_status(struct mmc_host *host, u32 *status);
int mmc_send_ext_csd(struct mmc_host *host, struct mmc_card *card, u8 *ext_csd);
// host communications functions
int send_cxd(struct mmc_host *host, u32 opcode, u32 arg, u32 flags, u32 *response);
int send_cxd_data(struct mmc_host *host, struct mmc_card *card, u32 opcode, u32 arg, u32 flags, u32 *response, void *buf, unsigned int len);
void set_chip_select(struct mmc_host *host, int mode);
static inline void set_ios(struct mmc_host *host);

void have_fun(struct mmc_host *host, struct mmc_card *card)
{
    struct msmsdcc_host *sdcchost;
    unsigned int status;
    int retval;
    u8 ext_csd[512];
    u32 response[4];
    int i;
    unsigned int pwr;
    void *virt;

    retval = mmc_send_ext_csd(host, card, ext_csd);
    status = getstatus(host);
    if(retval)
    {
	dmesg("wpthis - failed to get ext_csd. retval: %d, status: 0x%.8x\n", retval, status);
	return;
    }

    dmesg("wpthis - ext_csd: ");
    for(i = 0; i < 512; i++)
	dmesg("%.2x", ext_csd[i]);
    dmesg("\n");

    virt = ioremap(MSM_SDCC2_BASE, PAGE_SIZE);
    dmesg("wpthis - MSM_SDCC2_BASE remapped: 0x%.8x\n", (unsigned int)virt);

    // let's see if we can read the controller...
    pwr = readl((unsigned int)virt + MMCIPOWER);
    dmesg("wpthis - MMCIPOWER reg: 0x%.8x\n", pwr);

    pwr = MCI_PWR_OFF;

    dmesg("wpthis - MMCIPOWER (powered down): 0x%.8x\n", pwr);
    dmesg("wpthis - powering down sdcc2...\n");
    writel(pwr, (unsigned int)virt + MMCIPOWER);

    mmc_delay(1000); // 1 second? lol

    dmesg("wpthis - Powering up...\n");
    pwr = MCI_PWR_UP;
    writel(pwr, (unsigned int)virt + MMCIPOWER);

    pwr = readl((unsigned int)virt + MMCIPOWER);
    dmesg("wpthis - MMCIPOWER reg: 0x%.8x\n", pwr);

    mmc_delay(600);

    dmesg("wpthis - Powering on...\n");
    pwr = MCI_PWR_ON;
    writel(pwr, (unsigned int)virt + MMCIPOWER);

    pwr = readl((unsigned int)virt + MMCIPOWER);
    dmesg("wpthis - MMCIPOWER reg: 0x%.8x\n", pwr);

    iounmap(virt);
    dmesg("wpthis - MSM_SDCC2_BASE unmapped.\n");

    /*
    // these are stubs because i can't actually do it :(
    set_chip_select(host, MMC_CS_HIGH);
    mmc_delay(1);

    retval = send_cxd(host, 0, 0, MMC_RSP_NONE | MMC_CMD_BC, response);
    dmesg("wpthis - cmd0: %d\n", retval);

    set_chip_select(host, MMC_CS_DONTCARE);
    mmc_delay(1);

    retval = send_cxd(host, 1, 0, MMC_RSP_R3 | MMC_CMD_BCR, response);
    dmesg("wpthis - cmd1: %d, %.8x:%.8x:%.8x:%.8x\n", retval, response[0], response[1], response[2], response[3]);
    */

    return;
}

u32 getstatus(struct mmc_host *host)
{
    u32 response[4];
    int err;
    
    err = mmc_send_status(host, response);
    if(err)
	return 0xffffffff;
    return response[0];
}

int mmc_send_ext_csd(struct mmc_host *host, struct mmc_card *card, u8 *ext_csd)
{
    u32 response[4];

    return send_cxd_data(host, card, MMC_SEND_EXT_CSD, 0, MMC_RSP_R1 | MMC_CMD_ADTC, response, ext_csd, 512);
}

int mmc_send_status(struct mmc_host *host, u32 *response)
{
    return send_cxd(host, MMC_SEND_STATUS, RCA << 16, MMC_RSP_R1 | MMC_CMD_AC, response);
}

int send_cxd(struct mmc_host *host, u32 opcode, u32 arg, u32 flags, u32 *response)
{
    int err;
    struct mmc_command cmd;
    
    memset(&cmd, 0, sizeof(struct mmc_command));
    
    cmd.opcode = opcode;
    cmd.arg = arg;
    cmd.flags = flags;
    
    err = mmc_wait_for_cmd(host, &cmd, MMC_CMD_RETRIES);

    memcpy(response, cmd.resp, 4);
    
    return err;
}

void set_chip_select(struct mmc_host *host, int mode)
{
    return; // no crash kthx, bai.
    // this is destined to fail :(
    host->ios.chip_select = mode;
    set_ios(host);
}

static inline void set_ios(struct mmc_host *host)
{
    struct mmc_ios *ios = &host->ios;

    dmesg("%s: clock %uHz busmode %u powermode %u cs %u Vdd %u width %u timing %u\n",
	mmc_hostname(host), ios->clock, ios->bus_mode,
	ios->power_mode, ios->chip_select, ios->vdd,
	ios->bus_width, ios->timing);

    host->ops->set_ios(host, ios);
}

int send_cxd_data(struct mmc_host *host, struct mmc_card *card, u32 opcode, u32 arg, u32 flags, u32 *response, void *buf, unsigned int len)
{
    struct mmc_request mrq;
    struct mmc_command cmd;
    struct mmc_data data;
    struct scatterlist sg;
    void *data_buf;
    
    /* dma onto stack is unsafe/nonportable, but callers to this                                                                                                                                                                         
     * routine normally provide temporary on-stack buffers ...                                                                                                                                                                           
     */
    data_buf = kmalloc(len, GFP_KERNEL);
    if (data_buf == NULL)
	return -ENOMEM;
    
    memset(&mrq, 0, sizeof(struct mmc_request));
    memset(&cmd, 0, sizeof(struct mmc_command));
    memset(&data, 0, sizeof(struct mmc_data));
    
    mrq.cmd = &cmd;
    mrq.data = &data;
    
    cmd.opcode = opcode;
    cmd.arg = arg;
    cmd.flags = flags;
    
    data.blksz = len;
    data.blocks = 1;
    data.flags = MMC_DATA_READ;
    data.sg = &sg;
    data.sg_len = 1;
    
    sg_init_one(&sg, data_buf, len);
    
    if (opcode == MMC_SEND_CSD || opcode == MMC_SEND_CID) {
	/*                                                                                                                                                                                                                           
	 * The spec states that CSR and CID accesses have a timeout                                                                                                                                                                  
	 * of 64 clock cycles.                                                                                                                                                                                                       
	 */
	data.timeout_ns = 0;
	data.timeout_clks = 64;
    } else
	mmc_set_data_timeout(&data, card);

    mmc_wait_for_req(host, &mrq);
    
    memcpy(buf, data_buf, len);
    kfree(data_buf);

    memcpy(response, cmd.resp, 16);
    
    if (cmd.error)
	return cmd.error;
    if (data.error)
	return data.error;
    
    return 0;
}

u32 ops_search_table[7] = {
    0,
    0,
    0xc02ac578,
    0xc02aa750,
    0,
    0,
    0xc02aa6ac
};

static int __init wpthis_init(void)
{
    dev_t mmc0;
    struct block_device *bdev = 0;
    struct device *dev = 0;
    struct device *dev2 = 0;
    struct device *dev3 = 0;
    struct device *dev4 = 0;
    struct device *dev5 = 0;
    struct gendisk *gd = 0;
    struct mmc_host *host = 0;
    struct mmc_card *card = 0;
    struct hd_struct *part0 = 0;
    struct platform_device *pdev = 0;
    struct device_driver *driver = 0;
    char name[BDEVNAME_SIZE];
    int i;

    mmc0 = MKDEV(179, 0); // mmcblk0

    dmesg("wpthis - init\n");

    bdev = bdget(mmc0);
    dmesg("wpthis - bdev: 0x%.8x\n", (unsigned int)bdev);
    if(IS_ERR(bdev))
    {
	dmesg("wpthis - error opening 179:0\n");
	return -ENOSYS;
    }
    if(!bdev)
    {
	dmesg("wpthis - !bdev\n");
	return -ENOSYS;
    }
    bdevname(bdev, name);
    dmesg("wpthis - bdev %d:%d\n", MAJOR(bdev->bd_dev), MINOR(bdev->bd_dev));
    dmesg("wpthis - name: %s\n", name);

    gd = bdev->bd_disk;
    if(!gd)
    {
	dmesg("wpthis - !gd\n");
	return -ENOSYS;
    }
    dmesg("wpthis - gd: 0x%.8x\n", (unsigned int)gd);

    part0 = &gd->part0;
    if(!part0)
    {
	dmesg("wpthis - !part0\n");
	return -ENOSYS;
    }
    dmesg("wpthis - part0: 0x%.8x\n", (unsigned int)part0);
    
    dev = &part0->__dev;
    if(!dev)
    {
	dmesg("wpthis - !dev\n");
	return -ENOSYS;
    }
    dmesg("wpthis - dev: 0x%.8x\n", (unsigned int)dev);
    dmesg("wpthis - dev(dev_name): %s\n", dev_name(dev));

    dev2 = dev->parent;
    if(!dev2)
    {
	dmesg("wpthis - !dev2\n");
	return -ENOSYS;
    }
    dmesg("wpthis - dev2: 0x%.8x\n", (unsigned int)dev2);
    dmesg("wpthis - dev2(dev_name): %s\n", dev_name(dev2));

    dev3 = dev2->parent;
    if(!dev3)
    {
	dmesg("wpthis - !dev3\n");
	return -ENOSYS;
    }
    dmesg("wpthis - dev3: 0x%.8x\n", (unsigned int)dev3);
    dmesg("wpthis - dev3(dev_name): %s\n", dev_name(dev3));

    dev4 = dev3->parent;
    if(!dev4)
    {
	dmesg("wpthis - !dev4\n");
	return -ENOSYS;
    }
    dmesg("wpthis - dev4: 0x%.8x\n", (unsigned int)dev4);
    dmesg("wpthis - dev4(dev_name): %s\n", dev_name(dev4));

    dev5 = dev4->parent;
    if(!dev5)
    {
	dmesg("wpthis - !dev5\n");
	return -ENOSYS;
    }
    dmesg("wpthis - dev5: 0x%.8x\n", (unsigned int)dev5);
    dmesg("wpthis - dev5(dev_name): %s\n", dev_name(dev5));

    dmesg("wpthis - platform_bus: 0x%.8x\n", (unsigned int)&platform_bus);

    pdev = container_of(dev4, struct platform_device, dev);
    if(!pdev)
    {
	dmesg("wpthis - !pdev\n");
	return -ENOSYS;
    }
    dmesg("wpthis - pdev: 0x%.8x\n", (unsigned int)pdev);
    dmesg("wpthis - pdev->name: %s\n", pdev->name);
    dmesg("wpthis - pdev->id: %d\n", pdev->id);
    dmesg("wpthis - pdev->num_resources: %d\n", pdev->num_resources);

    for(i = 0; i < pdev->num_resources; i++)
    {
	if (pdev->resource[i].flags & IORESOURCE_MEM)
	    dmesg("wpthis - pdev mem resource: 0x%.8x\n", (unsigned int)&pdev->resource[i]);
	if (pdev->resource[i].flags & IORESOURCE_IRQ)
	    dmesg("wpthis - pdev irq resource: 0x%.8x\n", (unsigned int)&pdev->resource[i]);
	if (pdev->resource[i].flags & IORESOURCE_DMA)
	    dmesg("wpthis - pdev dma resource: 0x%.8x\n", (unsigned int)&pdev->resource[i]);
    }

    card = dev_to_mmc_card(dev2);
    host = card->host;
    if(!card)
    {
	dmesg("wpthis - !card\n");
	return -ENOSYS;
    }
    dmesg("wpthis - card: 0x%.8x\n", (unsigned int)card);
    dmesg("wpthis - host: 0x%.8x\n", (unsigned int)host);

    for(i = 0; i < sizeof(struct mmc_host); i += 4)
    {
	if(((unsigned int *)host)[i] == (unsigned int)card)
	{
	    if(!offset)
	    {
		dmesg("wpthis - host->card offset: %d, should be: %d, out of %d\n", i, offsetof(struct mmc_host, card), sizeof(struct mmc_host));
		offset = i;
	    }
	    else
	    {
		dmesg("wpthis - got a dupe host->card match.... i'm scared. bye.\n");
		return -ENOSYS;
	    }
	}
    }
    if(!offset)
    {
	dmesg("wpthis - never found host->card. bailing.\n");
	return -ENOSYS;
    }

    // it's GOT to be in here somewhere, right?
    /* apparently not.
    i = 0;
    while(1)
    {
	u32 base = ((unsigned int *)host)[i];
	dmesg("wpthis - i: %d\n", i);
	if(base == 0xc04214fc)
	{
	    dmesg("wpthis - GOT IT! at i: %d\n", i);
	    break;
	}
	i += 4;
    }
    */

    /*
    for(i = 0; i <= sizeof(struct mmc_host); i += 4)
    {
	u32 *base = (unsigned int *)((unsigned int *)host)[i];
	if(base > (unsigned int *)0xc0008000 && base < (unsigned int *)0xf0000000)
	{
	    int miss = 0;
	    int e;
	    for(e = 0; e < sizeof(ops_search_table) / 4; e++)
	    {
		u32 val = base[e];
		if(val != ops_search_table[e])
		    miss = 1;
		dmesg("wpthis - base: 0x%.8x, val: 0x%.8x, ops_tbl: 0x%.8x, miss: %d\n", (unsigned int)base, val, ops_search_table[e], miss);
	    }
	    if(!miss)
	    {
		dmesg("wpthis - found host_ops at 0x%.8x\n", (unsigned int)base);
	    }
	}
	else
	{
	    dmesg("wpthis - skipping 0x%.8x\n", (unsigned int)base);
	}
    }
    */

    dmesg("wpthis - mmc_hostname(host): %s\n", mmc_hostname(host));

    dmesg("wpthis - host->class_dev: 0x%.8x\n", (unsigned int)&host->class_dev);
    dmesg("wpthis - host->class_dev(dev_name): %s\n", dev_name(&host->class_dev));

    /*
    mmc_remove_host(host);

    return -ENOSYS;
    */

    dmesg("wpthis - claiming host...\n");
    mmc_claim_host(host);

    have_fun(host, card);
    
    dmesg("wpthis - releasing host...\n");
    mmc_release_host(host);

    return -ENOSYS;
}

static void __exit wpthis_exit(void)
{
    dmesg("wpthis - exit\n");
}

module_init(wpthis_init);
module_exit(wpthis_exit);
