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
 */

#ifndef __SSBI2_H_
#define __SSBI2_H_

#include <common.h>

/* PMIC arbiter1 SSBI registers. */
#define PA1_SSBI2_CMD (0x00500000)
#define PA1_SSBI2_RD_STATUS (0x00500004)

/* PMIC arbiter2 SSBI registers. */
#define PA2_SSBI2_CMD (0x00C00000)
#define PA2_SSBI2_RD_STATUS (0x00C00004)

/* SSBI2 command and status register fields */
#define SSBI2_CMD__RDWRN___M (0x01000000)
#define SSBI2_CMD__RDWRN___S (24)
#define SSBI2_CMD__REG_ADDR_14_8___M 0x007F0000
#define SSBI2_CMD__REG_ADDR_14_8___S (16)
#define SSBI2_CMD__REG_ADDR_7_0___M (0x0000FF00)
#define SSBI2_CMD__REG_ADDR_7_0___S (8)
#define SSBI2_CMD__REG_DATA___M (0x000000FF)
#define SSBI2_CMD__REG_DATA___S (0)
#define SSBI2_CMD___M (0x01FFFFFF)

#define SSBI2_RD_STATUS__TRANS_DONE___M (0x08000000)
#define SSBI2_RD_STATUS__TRANS_DONE___S (27)
#define SSBI2_RD_STATUS__TRANS_DENIED___M (0x04000000)
#define SSBI2_RD_STATUS__TRANS_DENIED___S (26)
#define SSBI2_RD_STATUS__TRANS_PROG___M (0x02000000)
#define SSBI2_RD_STATUS__TRANS_PROG___S (25)
#define SSBI2_RD_STATUS__RDWRN___M (0x01000000)
#define SSBI2_RD_STATUS__RDWRN___S (24)
#define SSBI2_RD_STATUS__REG_ADDR___M (0x007FFF00)
#define SSBI2_RD_STATUS__REG_ADDR___S (8)
#define SSBI2_RD_STATUS__REG_DATA___M (0x000000FF)
#define SSBI2_RD_STATUS__REG_DATA___S (0)

#define SSBI2_CMD_READ (1)
#define SSBI2_CMD_WRITE (0)
#define SSBI2_TIMEOUT_US (100)

/* SSBI2 register descriptor:
 * used to define the SSBI register addresses for different
 * SSBI2 instances on MSM.
 */
struct ssbi2_regs {
	u32 cmd;
	u32 status;
	u32 timeout;
};

/*SSBI slave ID, used to select different SSBI2 instances on MSM*/
enum ssbi2_slave {
	SSBI2_PA1 = 0,
	SSBI2_PA2
};

/* SSBI2 read and write routines provided by this module to the external world*/
int ssbi2_write(enum ssbi2_slave id, u8 *buffer, u32 length, u32 addr);
int ssbi2_read(enum ssbi2_slave id, u8 *buffer, u32 length, u32 addr);

#endif /* __SSBI2_H_ */
