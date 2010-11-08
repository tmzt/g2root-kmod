/*
 * Copyright (c) 2010, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * Initial version : Milind A Choudhary (milindc@codeaurora.org)
 */

/*
 * This module provides SSBI read/write interfaces to control
 * various functions on SSBI2 slave chips.
 * No initial cofiguration to setup the SSBI slave is performed
 * in this module. It is assumed that the earlier boot stages have
 * set the SSBI slave in a fully operational SSBI2 10bit addressing mode
 * and the slave is ready to accept commands.
 */

#include <common.h>
#include <asm/arch/ssbi2.h>

/*SSBI2 register descriptor for pmic arbiter1 */
static struct ssbi2_regs pa1_ssbi2_regs = {
	.cmd = PA1_SSBI2_CMD,
	.status = PA1_SSBI2_RD_STATUS,
	.timeout = SSBI2_TIMEOUT_US
};

/*SSBI2 register descriptor for pmic arbiter1 */
static struct ssbi2_regs pa2_ssbi2_regs = {
	.cmd = PA2_SSBI2_CMD,
	.status = PA2_SSBI2_RD_STATUS,
	.timeout = SSBI2_TIMEOUT_US
};

/*
 * Get the SSBI2 register descriptor for the given slave.
 * id => SSBI2 slave ID.
 */
struct ssbi2_regs *get_ssbi2_regs(enum ssbi2_slave id)
{
	struct ssbi2_regs *regs = NULL;

	switch (id)  {
	case SSBI2_PA1:
		regs = &pa1_ssbi2_regs;
		break;
	case SSBI2_PA2:
		regs = &pa2_ssbi2_regs;
		break;
	default:
		regs = NULL;
	}
	return regs;
}

/*
 * SSBI2 read : from any register on an SSBI slave.
 * id => SSBI slave ID, used to get SSBI slave register descriptor
 * buffer => Address of destination buffer.
 * length => Length of data to be read.
 * addr => The address to be read from the SSBI slave.
 *
 * Returns : (0) on success and (-1) on error.
 */
int ssbi2_read(enum ssbi2_slave id, u8 *buffer, u32 length, u32 addr)
{
	u32 val = 0x0;
	u32 temp = 0x0;
	u8 *buf = buffer;
	u32 timeout;
	struct ssbi2_regs *regs = NULL;

	regs = get_ssbi2_regs(id);
	if (regs == NULL)
		return -1;

	timeout = regs->timeout;

	while (length) {
		val |= ((addr << SSBI2_CMD__REG_ADDR_7_0___S) |
				(SSBI2_CMD_READ << SSBI2_CMD__RDWRN___S));
		IO_WRITE32(regs->cmd, val);

		while (!((temp = IO_READ32(regs->status)) &
				SSBI2_RD_STATUS__TRANS_DONE___M)) {
			if (--timeout == 0) {
				printf("%s: SSBI read failed , timeout\n", __func__);
				return -1;
			}
			udelay(1);
		}
		length--;
		*buf = (temp & (SSBI2_RD_STATUS__REG_DATA___M));
	}
	return 0;
}

/*
 * SSBI2 write : to any register on an SSBI slave.
 * id => SSBI slave ID, used to get SSBI slave register descriptor
 * buffer => Source address.
 * length => Length of data to be written.
 * addr => Destination address to be written to on the SSBI slave.
 *
 * Returns : (0) on success and (-1) on error.
 */
int ssbi2_write(enum ssbi2_slave id, u8 *buffer, u32 length, u32 addr)
{
	u32 val = 0x0;
	u32 temp = 0x0;
	u8 *buf = buffer;
	u32 timeout;
	struct ssbi2_regs *regs = NULL;

	regs = get_ssbi2_regs(id);
	if (regs == NULL)
		return -1;

	timeout = regs->timeout;
	while (length) {
		temp = 0x0;
		val = (addr << SSBI2_CMD__REG_ADDR_7_0___S) |
			(SSBI2_CMD_WRITE << SSBI2_CMD__RDWRN___S) |
			(*buf & SSBI2_CMD__REG_DATA___M);

		IO_WRITE32(regs->cmd, val);
		while (!((temp = IO_READ32(regs->status)) &
				SSBI2_RD_STATUS__TRANS_DONE___M)) {
			if (--timeout == 0) {
				printf("%s: SSBI write failed, timeout\n", __func__);
				return -1;
			}
			udelay(1);
		}
		length--;
		buf;
	}
	return 0;
}
