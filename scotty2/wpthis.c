#include <linux/module.h>
#include <linux/fs.h>
#include <linux/genhd.h>
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/mmc/mmc.h>
#include <linux/dma-mapping.h>
#include <linux/earlysuspend.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/delay.h>

#include <mach/dma.h>
#include <mach/gpio.h>
#include <mach/internal_power_rail.h>

#include "wpthis.h"
#include "msm_sdcc.h"
#include "proc_comm.h"

#define RCA 1
#define KSTART 0xc0008000
#define MSM_SDCC2_BASE 0xa0500000
#define MSM_SDCC_FMIN 144000L
#define MSM_SDCC_FMAX 50000000L
#define MSM_SDCC_4BIT 1
#define MMC_CMD_RETRIES 3

void gogogo(struct msmsdcc_host *sdcchost, struct mmc_host *mmchost, struct mmc_card *mmccard, struct clk *clk, struct clk *pclk);
void sdcc_writel(u32 data, unsigned int address, struct clk *clock);
int send_cxd(struct mmc_host *host, u32 opcode, u32 arg, u32 flags, u32 *response);
int send_cxd_data(struct mmc_host *host, struct mmc_card *card, u32 opcode, u32 arg, u32 flags, u32 *response, void *buf, unsigned int len);

static uint32_t emmc_wp_normal_table[] = {
    PCOM_GPIO_CFG(88, 0, GPIO_INPUT, GPIO_NO_PULL, GPIO_4MA),
    PCOM_GPIO_CFG(88, 2, GPIO_INPUT, GPIO_NO_PULL, GPIO_4MA)
};

static uint32_t emmc_wp_reset_table[] = {
    PCOM_GPIO_CFG(88, 0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_4MA),
    PCOM_GPIO_CFG(88, 2, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_4MA)
};

void gogogo(struct msmsdcc_host *sdcchost, struct mmc_host *mmchost, struct mmc_card *mmccard, struct clk *clk, struct clk *pclk)
{
    dmesg("Powering down eMMC...\n");
    config_gpio_table(emmc_wp_reset_table, ARRAY_SIZE(emmc_wp_reset_table));
    mdelay(1000);

    dmesg("Powering up eMMC...\n");
    config_gpio_table(emmc_wp_normal_table, ARRAY_SIZE(emmc_wp_normal_table));
    mdelay(1000);

    mmchost->bus_resume_flags |= MMC_BUSRESUME_NEEDS_RESUME;

    dmesg("Executing deferred resume...\n");
    mmc_resume_bus(mmchost);

}

void sdcc_writel(u32 data, unsigned int address, struct clk *clock)
{
    writel(data, address);
    udelay(1 + ((3 * USEC_PER_SEC) /
	    (clk_get_rate(clock) ? clk_get_rate(clock) : MSM_SDCC_FMIN)));
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

int send_cxd_data(struct mmc_host *host, struct mmc_card *card, u32 opcode, u32 arg, u32 flags, u32 *response, void *buf, unsigned int len)
{
    struct mmc_request mrq;
    struct mmc_command cmd;
    struct mmc_data data;
    struct scatterlist sg;
    void *data_buf;
    
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

static int __init wpthis_init(void)
{
    struct block_device *bdev = 0;
    struct gendisk *gdisk = 0;
    struct hd_struct *part0 = 0;

    struct device *block_dev = 0;
    struct device *card_dev = 0;
    struct device *host_dev = 0;
    struct device *sdcc_dev = 0;
    struct device *platform_dev = 0;

    struct mmc_card *mmccard = 0;
    struct mmc_host *mmchost = 0;
    struct msmsdcc_host *sdcchost = 0;

    struct clk *clk = 0;
    struct clk *pclk = 0;

    dmesg("init\n");

    get_or_die(bdev, lookup_bdev("/dev/block/mmcblk0"));
    get_or_die(gdisk, bdev->bd_disk);
    get_or_die(part0, &gdisk->part0);
    get_or_die(block_dev, &part0->__dev);
    get_or_die(card_dev, block_dev->parent);
    get_or_die(host_dev, card_dev->parent);
    get_or_die(sdcc_dev, host_dev->parent);
    get_or_die(platform_dev, sdcc_dev->parent);
    get_or_die(mmccard, container_of(card_dev, struct mmc_card, dev));
    get_or_die(mmchost, mmccard->host);
    get_or_die(sdcchost, (struct msmsdcc_host *)mmchost->private);
    get_or_die(clk, clk_get(sdcc_dev, "sdc_clk"));
    get_or_die(pclk, clk_get(sdcc_dev, "sdc_pclk"));

    print_dev(block_dev);
    print_dev(card_dev);
    print_dev(host_dev);
    print_dev(sdcc_dev);
    print_dev(platform_dev);
    print_clock(clk);
    print_clock(pclk);

    // make sure we have what we think we have.
    assert(!strcmp(dev_name(block_dev),"mmcblk0"));
    assert(!strcmp(dev_name(card_dev), "mmc0:0001"));
    assert(!strcmp(dev_name(host_dev), "mmc0"));
    assert(!strcmp(dev_name(sdcc_dev), "msm_sdcc.2"));

    mmc_claim_host(mmchost);
    clk_enable(pclk);
    clk_enable(clk);

    // alright, ready to go.
    gogogo(sdcchost, mmchost, mmccard, clk, pclk);
    // cleanup

    clk_disable(pclk);
    clk_disable(clk);
    mmc_release_host(mmchost);

 out:
    if(bdev && !IS_ERR(bdev))
	bdput(bdev);
    if(clk && !IS_ERR(clk))
	clk_put(clk);
    if(pclk && !IS_ERR(pclk))
	clk_put(pclk);

    // we don't actually want to stay in memory, we just want to do our business and get out.
    return -ENOSYS;
}

static void __exit wpthis_exit(void)
{
    dmesg("byebye\n");
}

module_init(wpthis_init);
module_exit(wpthis_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Scott Walker <walker.scott@gmail.com>");
MODULE_DESCRIPTION("i'd like this to disable wp on the emmc chip on my g2, please?");
