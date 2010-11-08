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
 * PMIC8901 and PMIC8085 supports SSBI/SSBI2/I2C interfaces.
 * Default mode is SSBI and the earlier boot stages configure the
 * SSBI slave on PMIC to bring it into SSBI2 10 bit address mode.
 * This module uses the SSBI read/write interfaces to configure
 * various functions on the PMIC.
 */

//include <common.h>
//include <asm/arch/ssbi2.h>
//include <asm/arch/pmic.h>

#include <ssbi2.h>
#include <pmic.h>

/*
 * Read from the control registers on PMIC via the SSBI2 interface.
 * buffer => Address of destination buffer.
 * length => Length of data to be read.
 * slave_addr => The address to be read from the PMIC.
 * SSBI2_PA2 selects PM8901 slave.
 * Returns : (0) on success and (-1) on error.
 */
int pm8901_read(u8 *buffer, u32 length, u32 slave_addr)
{
	return ssbi2_read(SSBI2_PA2, buffer, length, slave_addr);
}

/*
 * Write to the control registers on PMIC via the SSBI2 interface.
 * buffer => Source address.
 * length => Length of data to be written.
 * slave_addr => Destination address to be written to on the PMIC.
 * SSBI2_PA2 selects PM8901 slave.
 * Returns : (0) on success and (-1) on error.
 */
int pm8901_write(u8 *buffer, u32 length, u32 slave_addr)
{
	return ssbi2_write(SSBI2_PA2, buffer, length, slave_addr);
}

/*
 * Configure multi purpose pins on PMIC 8901 with given
 * type and voltage/current levels
 * mpp => MPP number (4 in total, PM8901_MPP{1-4})
 * type => digital/analog In/out, bidirectional etc
 * level => Voltage levels
 * control => misc config depending upon the type
 *
 * Returns : (0) on success and (-1) on error.
 */
int pm8901_mpp_config(u8 mpp, u8 type, u8 level, u8 control)
{

	u8 val = 0x0;

	if (mpp >= PM8901_MAX_MPPS)
		return -1;

	val  = (type << PM8901_MPP_TYPE___S) & PM8901_MPP_TYPE___M;
	val |= (level << PM8901_MPP_CONFIG_LVL___S) & PM8901_MPP_CONFIG_LVL___M;
	val |= (control << PM8901_MPP_CONFIG_CTL___S) & PM8901_MPP_CONFIG_CTL___M;

	return pm8901_write(&val, 1, PM8901_MPP_CONTROL(mpp));
}

/*
 * Turn on the 5V boost circuit (external to PMIC)
 * on 8660-FFA MPP1 is tied to the enable pin of boost block (active high)
 *
 * Returns : (0) on success and (-1) on error.
 */
int pm8901_5vboost_on(void)
{
	return pm8901_mpp_config(PM8901_MPP1, PM8901_MPP_TYPE_DIG_OUTPUT,
			PM8901_MPP_DIG_LEVEL_VPH, PM8901_MPP_DOUT_CTL_HIGH);
}

/*
 * Turn off the 5V boost circuit (external to PMIC)
 *
 * Returns : (0) on success and (-1) on error.
 */
int pm8901_5vboost_off(void)
{
	return pm8901_mpp_config(PM8901_MPP1, PM8901_MPP_TYPE_DIG_OUTPUT,
			PM8901_MPP_DIG_LEVEL_VPH, PM8901_MPP_DOUT_CTL_LOW);
}

/*
 * Enable the pin mask resource #17 in high power mode
 * Put the resource in HPM and leave the MODE_CONTROL,
 * CONTROL_PIN_MASKs in default masked state.
 *
 * Returns : (0) on success and (-1) on error.
 */
int pm8901_pmr17_on(void)
{
	u8 val = 0x00;
	pm8901_read(&val, 1, PM8901_PIN_MASK_RESOURCE(17));

	val &= ~PM8901_REG_PMR_STATE___M;
	val |= (PM8901_REG_PMR_STATE_HPM << PM8901_REG_PMR_STATE___S);

	return pm8901_write(&val, 1, PM8901_PIN_MASK_RESOURCE(17));
}

/*
 * Turn off the pin mask resource #17
 * Returns : (0) on success and (-1) on error.
 */
int pm8901_pmr17_off(void)
{
	u8 val = 0x00;
	pm8901_read(&val, 1, PM8901_PIN_MASK_RESOURCE(17));

	val &= ~PM8901_REG_PMR_STATE___M;
	val |= (PM8901_REG_PMR_STATE_OFF << PM8901_REG_PMR_STATE___S);

	return pm8901_write(&val, 1, PM8901_PIN_MASK_RESOURCE(17));
}

/*
 * Turn ON the OTG switch on PMIC8901 in order to provide VBUS to
 * the peripheral device when working in host mode.
 * By default the line is sourced at VPH_PWR (3.3 to 4V), an external
 * 5V boost circuit can be used to boost the line to 5V.
 *
 * Returns : (0) on success and (-1) on error.
 */
int usb_vbus_on(void)
{
	u8 val = 0x0;

	/*
	 * Ensure the OTG switch is in the default mode
	 * so that VBUS can be controlled by writing to PMR#17
	 */
	val = (OTG_SW_WIRE << PM8901_OTG_CONTROL_SW_EN___S);

	if (pm8901_write(&val, 1, PM8901_OTG_CONTROL) < 0) {
		printf("Unable to turn on OTG VBUS switch\n");
		return -1;
	}

	if (pm8901_5vboost_on() < 0) {
		printf("Unable to turn on 5V boost\n");
		return -1;
	}

	if (pm8901_pmr17_on() < 0) {
		printf("Unable to turn on USB VBUS regulator\n");
		return -1;
	}

	return 0;
}

/*
 * Turn off the OTG switch to pull the VBUS line down.
 * FIXME: For now turn the 5V boost off along with the OTG switch
 * and regulator. However the 5V boost may be used by other modules
 * as well, if so, we need to have a reference counted/vote driven
 * mechanism for controlling the 5V boost.
 *
 * Returns : (0) on success and (-1) on error.
 */
int usb_vbus_off(void)
{
	u8 val = 0x0;

	/* turn off the OTG switch and enable pulldown */
	val  = (OTG_SW_WIRE << PM8901_OTG_CONTROL_SW_EN___S);
	val |= (1 << PM8901_OTG_CONTROL_PD_EN___S);

	if (pm8901_write(&val, 1, PM8901_OTG_CONTROL) < 0) {
		printf("unable to turn off PM8901 OTG VBUS switch\n");
		return -1;
	}

	if (pm8901_pmr17_off() < 0) {
		printf("unable to turn off PM8901 regulator\n");
		return -1;
	}

	if (pm8901_5vboost_off() < 0) {
		printf("unable to turn off 5V boost \n");
		return -1;
	}

	return 0;
}

MODULE_EXPORT(pm8901_read);
MODULE_EXPORT(pm8901_write);
MODULE_EXPORT(pm8901_mpp_config);

