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

#ifndef __PMIC_H_
#define __PMIC_H_

//include <common.h>
#include <stddef.h>
#include <asm/types.h>


/* PMIC serial interface control registers */
#define PM8901_I2CSSBI_CNTRL (0x004)
#define PM8901_I2CSSBI_TEST (0x005)

/*  MPP Control registers */
#define PM8901_MPP_CONTROL_BASE (0x27)
#define PM8901_MPP_CONTROL(n) (PM8901_MPP_CONTROL_BASE + (n))

/*  MPP Type */
#define PM8901_MPP_TYPE___M (0xE0)
#define PM8901_MPP_TYPE___S (5)

/*  MPP Config Level */
#define PM8901_MPP_CONFIG_LVL___M (0x1C)
#define PM8901_MPP_CONFIG_LVL___S (2)

/*  MPP Config Control */
#define PM8901_MPP_CONFIG_CTL___M (0x03)
#define PM8901_MPP_CONFIG_CTL___S (0)

/*  MPP Type */
#define PM8901_MPP_TYPE_DIG_INPUT       (0)
#define PM8901_MPP_TYPE_DIG_OUTPUT      (1)
#define PM8901_MPP_TYPE_DIG_BI_DIR      (2)
#define PM8901_MPP_TYPE_ANA_INPUT       (3)
#define PM8901_MPP_TYPE_ANA_OUTPUT      (4)
#define PM8901_MPP_TYPE_SINK            (5)
#define PM8901_MPP_TYPE_DTEST_SINK      (6)
#define PM8901_MPP_TYPE_DTEST_OUTPUT    (7)

/* Digital Input/Output: level */
#define PM8901_MPP_DIG_LEVEL_MSMIO  (0)
#define PM8901_MPP_DIG_LEVEL_DIG    (1)
#define PM8901_MPP_DIG_LEVEL_L5     (2)
#define PM8901_MPP_DIG_LEVEL_S4     (3)
#define PM8901_MPP_DIG_LEVEL_VPH    (4)

/*  Digital Input: control */
#define PM8901_MPP_DIN_TO_INT       (0)
#define PM8901_MPP_DIN_TO_DBUS1     (1)
#define PM8901_MPP_DIN_TO_DBUS2     (2)
#define PM8901_MPP_DIN_TO_DBUS3     (3)

/*  Digital Output: control */
#define PM8901_MPP_DOUT_CTL_LOW      (0)
#define PM8901_MPP_DOUT_CTL_HIGH     (1)
#define PM8901_MPP_DOUT_CTL_MPP      (2)
#define PM8901_MPP_DOUT_CTL_INV_MPP  (3)

/* List of MPPs on PMIC8901 */
#define PM8901_MPP1     (0)
#define PM8901_MPP2     (1)
#define PM8901_MPP3     (2)
#define PM8901_MPP4     (3)
#define PM8901_MAX_MPPS (4)

/*  Pin mask resource registers */
#define PM8901_PIN_MASK_RESOURCE_BASE (0x0A6)
#define PM8901_PIN_MASK_RESOURCE(n) (PM8901_PIN_MASK_RESOURCE_BASE + (n))

#define PM8901_REG_PMR_STATE___M     (0x60)
#define PM8901_REG_PMR_STATE___S     (5)
#define PM8901_REG_PMR_STATE_HPM     (0x3)
#define PM8901_REG_PMR_STATE_LPM     (0x2)
#define PM8901_REG_PMR_STATE_OFF     (0x1)

/* OTG switch control registers on PMIC8901 */
#define PM8901_OTG_CONTROL (0x55)
#define PM8901_OTG_CONTROL_SW_EN___M (0xC0)
#define PM8901_OTG_CONTROL_SW_EN___S (6)
#define OTG_SW_OFF (0x0)
#define OTG_SW_ON  (0x1)
#define OTG_SW_WIRE (0x2)

#define PM8901_OTG_CONTROL_PD_EN___M (0x40)
#define PM8901_OTG_CONTROL_PD_EN___S (5)

#define PM8901_OTG_TEST (0x56)
#define PM8901_OTG_TRIM (0x32D)

/* Turn the USB VBUS on/off by configuring the appropriate PMIC*/
int usb_vbus_on(void);
int usb_vbus_off(void);

#endif /* __PMIC_H_ */
