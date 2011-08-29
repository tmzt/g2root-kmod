/*
    Copyright (C) 2011  Guhl
    This work is heavily based on the module wpthis by scotty2

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
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

#include "msm_sdcc.h"
#include "proc_comm.h"
#include "gfmod.h"

#define RCA 1
#define MSM_SDCC_FMIN 144000L
#define MSM_SDCC_FMAX 50000000L
#define MMC_CMD_RETRIES        3

void __iomem *io_ptr;

int gogogo(struct msmsdcc_host *sdcchost, struct mmc_host *mmchost, struct mmc_card *mmccard, struct clk *clk, struct clk *pclk);
int check_wp(struct mmc_host *host, struct mmc_card *card, uint32_t total_sectors, uint32_t wp_grp_size);
int toggle_io(int toggle);
int reset_and_init_emmc(struct mmc_host *mmchost, struct mmc_card *card, struct clk *clk, uint32_t *ocr, uint32_t *cid, uint32_t *csd, uint8_t *ext_csd);
void powercycle_emmc(void);
void deferred_resume(struct mmc_host *host);
int send_cxd(struct mmc_host *host, u32 opcode, u32 arg, u32 flags, u32 *response);
int send_cxd_data(struct mmc_host *host, struct mmc_card *card, u32 opcode, u32 arg, u32 flags, u32 *response, void *buf, unsigned int len);
int send_ext_csd(struct mmc_card *card, u8 *ext_csd);
int send_status(struct mmc_card *card, u32 *status);
int mmc_switch(struct mmc_card *card, u8 set, u8 index, u8 value);

static inline void mmc_delay(unsigned int ms)
{
	if (ms < 1000 / HZ) {
		cond_resched();
		mdelay(ms);
	} else {
		msleep(ms);
	}
}

int gogogo(struct msmsdcc_host *sdcchost, struct mmc_host *mmchost, struct mmc_card *mmccard, struct clk *clk, struct clk *pclk)
{
    uint32_t ocr;
    uint32_t cid[4];
    uint32_t csd[4];
    uint8_t ext_csd[512];
    int kernel_reinit;
    uint8_t wp_grp_size;
    uint8_t erase_grp_size;
    uint8_t erase_grp_mult;
    uint32_t wp_grp_size_real;
    uint32_t sector_count;
    uint32_t total_wp_groups;
    int i;
    int retval;

    kernel_reinit = 0;

    if(reset_and_init_emmc(mmchost, mmccard, clk, &ocr, cid, csd, ext_csd)) {
		// get wp information.
		// wp_grp_size is the amount of erase groups - 1 consist of a write protect group.
		// a write protect grp is (erase_grp_size + 1) * (erase_grp_mult + 1) write blocks (512B).

		dmesg("cid: ");
		for(i = 0; i < 4; i++)
			printk("%.8x", cid[i]);
		printk("\n");

		dmesg("csd: ");
		for(i = 0; i < 4; i++)
			printk("%.8x", csd[i]);
		printk("\n");

		dmesg("ext_csd: ");
		for(i = 0; i < 512; i++)
			printk("%.2x", ext_csd[i]);
		printk("\n");

		wp_grp_size = csd[2] & 0x1f;
		erase_grp_size = (csd[2] >> 10) & 0x1f;
		erase_grp_mult = (csd[2] >> 5) & 0x1f;
		wp_grp_size_real = ((erase_grp_size + 1) * (erase_grp_mult + 1)) * (wp_grp_size + 1);
		dmesg("Write protect group size: %lu sectors\n", (unsigned long)wp_grp_size_real);

		memcpy(&sector_count, &ext_csd[212], 4);

		dmesg("Total sectors: %lu\n", (unsigned long)sector_count);

		total_wp_groups = sector_count / wp_grp_size_real;

		dmesg("Total write protect groups: %lu\n", (unsigned long)total_wp_groups);

		retval = check_wp(mmchost, mmccard, sector_count, wp_grp_size_real);
		if(retval == -1) {
			dmesg("Failed to detect write-protect status.\n");
		} else if(retval == 1) {
			dmesg("Write protect is enabled.\n");
		} else {
			dmesg("Write protect is disabled already returning MOD_RET_NONEED.\n");
			return MOD_RET_NONEED;
//			dmesg("Write protect is disabled already, continuing anyway.\n");
		}
	} else {
		dmesg("Manual soft-reset failed, will fall back to kernel re-init after power-cycle, and won't check WP status.\n");
		kernel_reinit = 1;
	}

	dmesg("Power-cycling eMMC by io ...\n");
	powercycle_emmc();

	dmesg("Re-initializing eMMC...\n");
	if(kernel_reinit) {
		deferred_resume(mmchost);
		return MOD_RET_FAILWP;
	} else {
		if(!reset_and_init_emmc(mmchost, mmccard, clk, &ocr, cid, csd, ext_csd)) {
			dmesg("Manual re-init failed, falling back to kernel re-init.\n");
			powercycle_emmc();
			deferred_resume(mmchost);
		}
		// verify wp.
		retval = check_wp(mmchost, mmccard, sector_count, wp_grp_size_real);
		if(retval == -1){
			dmesg("Failed to verify write-protect disabled.\n");
			return MOD_RET_FAILWP;
		} else if(retval == 1) {
			dmesg("Failed to disable write-protect :(\n");
			return MOD_RET_FAIL;
		}
		dmesg("Write protect disabled. :)\n");
    }

    return MOD_RET_OK;
}

int check_wp(struct mmc_host *host, struct mmc_card *card, uint32_t total_sectors, uint32_t wp_grp_size)
{
    int i;
    uint32_t wp_groups;
    uint32_t wpbits[2];
    uint32_t response[4];

    wp_groups = total_sectors / wp_grp_size;

    for(i = 0; i < wp_groups; i += 32)
    {
	if(send_cxd_data(host, card, 31, i * wp_grp_size, MMC_CMD_ADTC | MMC_RSP_R1, response, wpbits, 8))
	    return -1;
	if(wpbits[0] || wpbits[1])
	    return 1;
    }
    return 0;
}

void powercycle_emmc()
{
	uint32_t cpio_table;
	mdelay(200);
	cpio_table = 0x64580;
	config_gpio_table(&cpio_table, 1);
	dmesg("powercycle_emmc:set gpio 88 direction output\n");
	gpio_direction_output(88, 0);
	dmesg("powercycle_emmc: configure gpio 88\n");
	gpio_configure(88, 0);
	dmesg("powercycle_emmc: set gpio 88 to 0\n");
	gpio_set_value(88, 0);
	mdelay(200);
	dmesg("powercycle_emmc: set gpio 88 to 1\n");
	gpio_set_value(88, 1);
	mdelay(200);

	dmesg("powercycle_emmc: toggle io to 0\n");
	toggle_io(0);
	mdelay(550);
    dmesg("powercycle_emmc: toggle io to 1\n");
	toggle_io(1);
	mdelay(350);
}

int toggle_io(int toggle){
	ONDEBUG(dmesg("toggle_io on entry, toggle=%d\n",toggle));
	ONDEBUG(print_hex_dump(KERN_DEBUG, "io_ptr+268: ", DUMP_PREFIX_ADDRESS,16, 1, io_ptr + 268, 16, true));
	ONDEBUG(print_hex_dump(KERN_DEBUG, "io_ptr+8: ", DUMP_PREFIX_ADDRESS,16, 1, io_ptr + 8, 16, true));
    iowrite32(ioread32(io_ptr + 268) & 0xFFEFFFF, io_ptr + 268);
    iowrite32(88,io_ptr + 8);
    iowrite32(0, io_ptr + 9);
	if (toggle == 0) {
		iowrite32(ioread32(io_ptr + 8) & 0xFFEFFFF, io_ptr + 8);
	} else {
		iowrite32(ioread32(io_ptr + 8) | 0x100000, io_ptr + 8);
	}
	ONDEBUG(dmesg("toggle_io on exit\n"));
	ONDEBUG(print_hex_dump(KERN_DEBUG, "io_ptr+268: ", DUMP_PREFIX_ADDRESS,16, 1, io_ptr + 268, 16, true));
	ONDEBUG(print_hex_dump(KERN_DEBUG, "io_ptr+8: ", DUMP_PREFIX_ADDRESS,16, 1, io_ptr + 8, 16, true));
    return 1;
}

int reset_and_init_emmc(struct mmc_host *host, struct mmc_card *card, struct clk *clk, uint32_t *ocr, uint32_t *cid, uint32_t *csd, uint8_t *ext_csd)
{
    int attempt;
    uint32_t response[4];

    dmesg("reset and init the mmc\n");
    assert(!send_cxd(host, 0, 0xF0F0F0F0u, 0x40u, response));
    msleep(200);
    // CMD0 will put the card into the idle state. card will expect clock to be minimum mmc spec.
    dmesg("set mmc idle\n");
    assert(!send_cxd(host, 0, 0, MMC_CMD_BC | MMC_RSP_NONE, response));
    // set clock.
    dmesg("set clock\n");
    assert(!clk_set_rate(clk, MSM_SDCC_FMIN));
    // we have to wait for the busy bit (31) to go high on the ocr to indicate that the card is done booting.
    // card will come out of this mode in the ready state.
    *ocr = 0;
    attempt = 0;
    dmesg("wait for card to come ready\n");
    while(!(*ocr & 0x80000000))
    {
        attempt++;
        assert(!send_cxd(host, 1, *ocr, MMC_CMD_BCR | MMC_RSP_R3, response));
        *ocr = response[0];

	assert(attempt != 100);
    }
    dmesg("card ready\n");

    // cmd2 will get the cid, and put the card into identification mode.
    dmesg("get card cid\n");
    assert(!send_cxd(host, 2, 0, MMC_CMD_BCR | MMC_RSP_R2, cid));

    // cmd3 will set the rca, and put the card into standby.
    dmesg("set card standby\n");
    assert(!send_cxd(host, 3, RCA << 16, MMC_CMD_AC | MMC_RSP_R1, response));
    // cmd9 will get the csd.
    dmesg("get card csd\n");
    assert(!send_cxd(host, 9, RCA << 16, MMC_CMD_AC | MMC_RSP_R2, csd));
    mdelay(100); // not sure why this is required here, but next command fails if it's not.
    // TODO rework that code
    // cmd7 will put the card into transfer mode.
    dmesg("put the card into transfer mode\n");
    assert(!send_cxd(host, 7, RCA << 16, MMC_CMD_AC | MMC_RSP_R1, response));

    // byte 185 is the high-speed mode byte of the ext_csd, 1 is high-speed.
    dmesg("put the card into high speed mode\n");
    assert(!send_cxd(host, 6, 0x03b90100, MMC_CMD_AC | MMC_RSP_R1B, response));
    // r1b response type appears broken for some reason... guess i have to do it manually.
    response[0] = 0;
    attempt = 0;
    while(!(response[0] & 0x00000100))
    {
        attempt++;
        assert(!send_cxd(host, 13, RCA << 16, MMC_CMD_AC | MMC_RSP_R1, response));

        assert(attempt != 100);
    }
    // verify switch was successful.
    assert(!(response[0] & 0x00000080));
    dmesg("switch into high speed mode successful\n");

    // byte 183 is the bus-width byte of the ext_csd, 6 is 8-bit sdr.
    assert(!send_cxd(host, 6, 0x03b70200, MMC_CMD_AC | MMC_RSP_R1B, response));
    response[0] = 0;
    attempt = 0;
    while(!(response[0] & 0x00000100))
    {
	attempt++;
	assert(!send_cxd(host, 13, RCA << 16, MMC_CMD_AC | MMC_RSP_R1, response));

	assert(attempt != 100);
    }
    assert(!(response[0] & 0x00000080));

    // the device is set for high speed operation, we can jack up the clock rate now.
    dmesg("device is set for high speed operation - jack up the clock rate\n");
    assert(!clk_set_rate(clk, MSM_SDCC_FMAX));

    // cmd8 will get the ext_csd.
    dmesg("get ext_csd\n");
    assert(!send_cxd_data(host, card, 8, 0, MMC_CMD_ADTC | MMC_RSP_R1, response, ext_csd, 512));

    // TODO end rework
    return 1;

out:
	return 0;
}

int send_status(struct mmc_card *card, u32 *status)
{
	int err;
	struct mmc_command cmd;

	BUG_ON(!card);
	BUG_ON(!card->host);

	memset(&cmd, 0, sizeof(struct mmc_command));

	cmd.opcode = MMC_SEND_STATUS;
	if (!mmc_host_is_spi(card->host))
		cmd.arg = card->rca << 16;
	cmd.flags = MMC_RSP_SPI_R2 | MMC_RSP_R1 | MMC_CMD_AC;

	err = mmc_wait_for_cmd(card->host, &cmd, MMC_CMD_RETRIES);
	if (err)
		return err;

	/* NOTE: callers are required to understand the difference
	 * between "native" and SPI format status words!
	 */
	if (status)
		*status = cmd.resp[0];

	return 0;
}

int mmc_switch(struct mmc_card *card, u8 set, u8 index, u8 value)
{
	int err;
	int retries = 3;
	struct mmc_command cmd;
	u32 status;
	unsigned long delay = jiffies + HZ;

	BUG_ON(!card);
	BUG_ON(!card->host);

	memset(&cmd, 0, sizeof(struct mmc_command));

	cmd.opcode = MMC_SWITCH;
	cmd.arg = (MMC_SWITCH_MODE_WRITE_BYTE << 24) |
		  (index << 16) |
		  (value << 8) |
		  set;
	cmd.flags = MMC_RSP_SPI_R1B | MMC_RSP_R1B | MMC_CMD_AC;

	err = mmc_wait_for_cmd(card->host, &cmd, MMC_CMD_RETRIES);
	if (err)
		return err;

	mmc_delay(1);
	/* Must check status to be sure of no errors */
	do {
		err = send_status(card, &status);

		if (err) {
			printk(KERN_ERR "%s: failed to get status (%d)\n", __func__, err);
			mmc_delay(5);
			retries--;
			continue;
		}

		if (card->host->caps & MMC_CAP_WAIT_WHILE_BUSY)
			break;
		if (mmc_host_is_spi(card->host))
			break;

		if (time_after(jiffies, delay)) {
			printk(KERN_ERR "failed to get card ready!!!\n");
			break;
		}
	} while (retries && R1_CURRENT_STATE(status) == 7);

	if (mmc_host_is_spi(card->host)) {
		if (status & R1_SPI_ILLEGAL_COMMAND)
			return -EBADMSG;
	} else {
		if (status & 0xFDFFA000)
			printk(KERN_WARNING "%s: unexpected status %#x after "
			       "switch", mmc_hostname(card->host), status);
		if (status & R1_SWITCH_ERROR)
			return -EBADMSG;
	}

	return 0;
}

int send_ext_csd(struct mmc_card *card, u8 *ext_csd)
{
	uint32_t response[4];
		return send_cxd_data(card->host, card, MMC_SEND_EXT_CSD, 0, MMC_RSP_R1 | MMC_CMD_ADTC,
				response, ext_csd, 512);
//	return send_cxd_data(card->host, card, MMC_SEND_EXT_CSD, 0, MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC,
//			response, ext_csd, 512);
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

void deferred_resume(struct mmc_host *mmchost)
{
    mmc_resume_host(mmchost);
    mmc_resume_bus(mmchost);
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

    memcpy(response, cmd.resp, 16);

    return err;
}

static int __init gfmod_init(void)
{
    int retval;

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

	char *mybuf;
    char *mylinebuf;

	dmesg("init\n");

	mybuf = vmalloc(4096);
	mylinebuf = vmalloc(4096 * 4);
	io_ptr = ioremap(0xABC00000u, 4096);
	ONDEBUG(dmesg("io_ptr = 0x%.8x\n",(unsigned int)io_ptr));
    ONDEBUG(print_hex_dump(KERN_DEBUG, "io_ptr: ", DUMP_PREFIX_ADDRESS,16, 1, io_ptr, 300, true));

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
    assert(strstr(dev_name(block_dev),"mmcblk"));
    assert(strstr(dev_name(card_dev), "mmc"));
    assert(strstr(dev_name(host_dev), "mmc"));
    assert(strstr(dev_name(sdcc_dev), "msm_sdcc"));

    mmc_claim_host(mmchost);
    clk_enable(pclk);
    clk_enable(clk);

    // alright, ready to go.
    retval = gogogo(sdcchost, mmchost, mmccard, clk, pclk);
    // cleanup

    clk_disable(pclk);
    clk_disable(clk);
    mmc_release_host(mmchost);

	iounmap(io_ptr);
    return retval;

 out:
    if(bdev && !IS_ERR(bdev))
	bdput(bdev);
    if(clk && !IS_ERR(clk))
	clk_put(clk);
    if(pclk && !IS_ERR(pclk))
	clk_put(pclk);

    // we don't actually want to stay in memory, we just want to do our business and get out.
    return MOD_RET_FAILINIT;
}

static void __exit gfmod_exit(void)
{
    dmesg("byebye\n");
}


module_init(gfmod_init);
module_exit(gfmod_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Guhl");
MODULE_DESCRIPTION("i'd like this to disable wp on the emmc chip, please?");
