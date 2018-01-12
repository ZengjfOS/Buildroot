/*
 * Copyright (C) 2013 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include "mxc_hdmi.h"

static hdmi_data_info_s hdmi_instance = { 0 };

/*!
 * write bit fields of hdmi internal regiters
 *
 * @param  	data:	written value of the bit fields
 * @param  	addr:	address of the register
 * @param  	shift:	lsb offset of the bit-field
 * @param  	width:	width of the bit-field
 */
static void writebf(unsigned char data, unsigned int addr, unsigned char shift, unsigned char width)
{
	unsigned char temp = 0;
	unsigned char mask = (1 << width) - 1;
	if (data > mask) {
		printf("Invalid parameter for writebf.\n");
		return;
	}

	temp = readb(addr);
	temp &= ~(mask << shift);
	temp |= (data & mask) << shift;
	writeb(temp, addr);
}

static int isColorSpaceConversion(hdmi_data_info_s * hdmi)
{
	return (hdmi->enc_in_format != hdmi->enc_out_format) ? TRUE : FALSE;
}

static int isColorSpaceDecimation(hdmi_data_info_s * hdmi)
{
	return ((hdmi->enc_out_format == eYCC422_8bits) &&
		(hdmi->enc_in_format == eRGB
		|| hdmi->enc_in_format == eYCC444)) ? TRUE : FALSE;
}

static int isColorSpaceInterpolation(hdmi_data_info_s * hdmi)
{
	return ((hdmi->enc_in_format == eYCC422_8bits) &&
		(hdmi->enc_out_format == eRGB
		|| hdmi->enc_out_format == eYCC444)) ? TRUE : FALSE;
}

/*!
 * update the color space conversion coefficients.
 */
static void update_csc_coeffs(hdmi_data_info_s * hdmi)
{
	unsigned short csc_coeff[3][4];
	unsigned int csc_scale = 1;
	u8 val;
	int coeff_selected = FALSE;

	memset(&csc_coeff, 0, sizeof(csc_coeff));

	if (isColorSpaceConversion(hdmi) == TRUE)   // csc needed
	{
		if (hdmi->enc_out_format == eRGB) {
			if (hdmi->colorimetry == eITU601) {
				csc_coeff[0][0] = 0x2000;
				csc_coeff[0][1] = 0x6926;
				csc_coeff[0][2] = 0x74fd;
				csc_coeff[0][3] = 0x010e;

				csc_coeff[1][0] = 0x2000;
				csc_coeff[1][1] = 0x2cdd;
				csc_coeff[1][2] = 0x0000;
				csc_coeff[1][3] = 0x7e9a;

				csc_coeff[2][0] = 0x2000;
				csc_coeff[2][1] = 0x0000;
				csc_coeff[2][2] = 0x38b4;
				csc_coeff[2][3] = 0x7e3b;

				csc_scale = 1;
				coeff_selected = TRUE;
			} else if (hdmi->colorimetry == eITU709) {
				csc_coeff[0][0] = 0x2000;
				csc_coeff[0][1] = 0x7106;
				csc_coeff[0][2] = 0x7a02;
				csc_coeff[0][3] = 0x00a7;

				csc_coeff[1][0] = 0x2000;
				csc_coeff[1][1] = 0x3264;
				csc_coeff[1][2] = 0x0000;
				csc_coeff[1][3] = 0x7e6d;

				csc_coeff[2][0] = 0x2000;
				csc_coeff[2][1] = 0x0000;
				csc_coeff[2][2] = 0x3b61;
				csc_coeff[2][3] = 0x7e25;

				csc_scale = 1;
				coeff_selected = TRUE;
			}
		} else if (hdmi->enc_in_format == eRGB) {
			if (hdmi->colorimetry == eITU601) {
				csc_coeff[0][0] = 0x2591;
				csc_coeff[0][1] = 0x1322;
				csc_coeff[0][2] = 0x074b;
				csc_coeff[0][3] = 0x0000;

				csc_coeff[1][0] = 0x6535;
				csc_coeff[1][1] = 0x2000;
				csc_coeff[1][2] = 0x7acc;
				csc_coeff[1][3] = 0x0200;

				csc_coeff[1][0] = 0x6acd;
				csc_coeff[1][1] = 0x7534;
				csc_coeff[1][2] = 0x2000;
				csc_coeff[1][3] = 0x0200;

				csc_scale = 0;
				coeff_selected = TRUE;
 			} else if (hdmi->colorimetry == eITU709) {
				csc_coeff[0][0] = 0x2dc5;
				csc_coeff[0][1] = 0x0d9b;
				csc_coeff[0][2] = 0x049e;
				csc_coeff[0][3] = 0x0000;

				csc_coeff[1][0] = 0x63f0;
				csc_coeff[1][1] = 0x2000;
				csc_coeff[1][2] = 0x7d11;
				csc_coeff[1][3] = 0x0200;

				csc_coeff[2][0] = 0x6756;
				csc_coeff[2][1] = 0x78ab;
				csc_coeff[2][2] = 0x2000;
				csc_coeff[2][3] = 0x0200;

				csc_scale = 0;
				coeff_selected = TRUE;
			}
		}
	}

	if (!coeff_selected) {
		csc_coeff[0][0] = 0x2000;
		csc_coeff[0][1] = 0x0000;
		csc_coeff[0][2] = 0x0000;
		csc_coeff[0][3] = 0x0000;

		csc_coeff[1][0] = 0x0000;
		csc_coeff[1][1] = 0x2000;
		csc_coeff[1][2] = 0x0000;
		csc_coeff[1][3] = 0x0000;

		csc_coeff[2][0] = 0x0000;
		csc_coeff[2][1] = 0x0000;
		csc_coeff[2][2] = 0x2000;
		csc_coeff[2][3] = 0x0000;

		csc_scale = 1;
	}

	/* Update CSC parameters in HDMI CSC registers */
	writeb((unsigned char)(csc_coeff[0][0] & 0xFF),
		HDMI_CSC_COEF_A1_LSB);
	writeb((unsigned char)(csc_coeff[0][0] >> 8),
		HDMI_CSC_COEF_A1_MSB);
	writeb((unsigned char)(csc_coeff[0][1] & 0xFF),
		HDMI_CSC_COEF_A2_LSB);
	writeb((unsigned char)(csc_coeff[0][1] >> 8),
		HDMI_CSC_COEF_A2_MSB);
	writeb((unsigned char)(csc_coeff[0][2] & 0xFF),
		HDMI_CSC_COEF_A3_LSB);
	writeb((unsigned char)(csc_coeff[0][2] >> 8),
		HDMI_CSC_COEF_A3_MSB);
	writeb((unsigned char)(csc_coeff[0][3] & 0xFF),
		HDMI_CSC_COEF_A4_LSB);
	writeb((unsigned char)(csc_coeff[0][3] >> 8),
		HDMI_CSC_COEF_A4_MSB);

	writeb((unsigned char)(csc_coeff[1][0] & 0xFF),
		HDMI_CSC_COEF_B1_LSB);
	writeb((unsigned char)(csc_coeff[1][0] >> 8),
		HDMI_CSC_COEF_B1_MSB);
	writeb((unsigned char)(csc_coeff[1][1] & 0xFF),
		HDMI_CSC_COEF_B2_LSB);
	writeb((unsigned char)(csc_coeff[1][1] >> 8),
		HDMI_CSC_COEF_B2_MSB);
	writeb((unsigned char)(csc_coeff[1][2] & 0xFF),
		HDMI_CSC_COEF_B3_LSB);
	writeb((unsigned char)(csc_coeff[1][2] >> 8),
		HDMI_CSC_COEF_B3_MSB);
	writeb((unsigned char)(csc_coeff[1][3] & 0xFF),
		HDMI_CSC_COEF_B4_LSB);
	writeb((unsigned char)(csc_coeff[1][3] >> 8),
		HDMI_CSC_COEF_B4_MSB);

	writeb((unsigned char)(csc_coeff[2][0] & 0xFF),
		HDMI_CSC_COEF_C1_LSB);
	writeb((unsigned char)(csc_coeff[2][0] >> 8),
		HDMI_CSC_COEF_C1_MSB);
	writeb((unsigned char)(csc_coeff[2][1] & 0xFF),
		HDMI_CSC_COEF_C2_LSB);
	writeb((unsigned char)(csc_coeff[2][1] >> 8),
		HDMI_CSC_COEF_C2_MSB);
	writeb((unsigned char)(csc_coeff[2][2] & 0xFF),
		HDMI_CSC_COEF_C3_LSB);
	writeb((unsigned char)(csc_coeff[2][2] >> 8),
		HDMI_CSC_COEF_C3_MSB);
	writeb((unsigned char)(csc_coeff[2][3] & 0xFF),
		HDMI_CSC_COEF_C4_LSB);
	writeb((unsigned char)(csc_coeff[2][3] >> 8),
		HDMI_CSC_COEF_C4_MSB);

	val = readb(HDMI_CSC_SCALE);
	val &= ~HDMI_CSC_SCALE_CSCSCALE_MASK;
	val |= csc_scale & HDMI_CSC_SCALE_CSCSCALE_MASK;
	writeb(val, HDMI_CSC_SCALE);
}

/*!
 * configure the interrupt mask of source PHY.
 */
static void hdmi_phy_int_mask(int mask)
{
	writeb(mask, HDMI_PHY_MASK0);
}

/*!
 * set the DE polarity
 * @param	pol: high or low for DE active
 */
static void hdmi_phy_sel_data_en_pol(int pol)
{
	writebf(pol, HDMI_PHY_CONF0, HDMI_PHY_CONF0_SELDATAENPOL_OFFSET, 1);
}

/*!
 * select the interface control??
 * @param  seldipif: interface selection
 */
static void hdmi_phy_sel_interface_control(int seldipif)
{
	writebf(seldipif, HDMI_PHY_CONF0, HDMI_PHY_CONF0_SELDIPIF_OFFSET, 1);
}

/*!
 * enable TMDS output
 * @param	en: enable or disable the TMDS output
 */
static void hdmi_phy_enable_tmds(int en)
{
	writebf(en, HDMI_PHY_CONF0, HDMI_PHY_CONF0_ENTMDS_OFFSET, 1);
}

/*!
 * PHY power down enable
 * @param 	en: enable or disable PHY
 * 1 - power down disable
 * 0 - power down enable
 */
static void hdmi_phy_enable_power(int en)
{
	writebf(en, HDMI_PHY_CONF0, HDMI_PHY_CONF0_PDZ_OFFSET, 1);
}

static inline void hdmi_phy_test_clear(unsigned char bit)
{
	writebf(bit, HDMI_PHY_TST0, HDMI_PHY_TST0_TSTCLR_OFFSET, 1);
}

static void hdmi_phy_gen2_txpwron(unsigned char enable)
{
	writebf(enable, HDMI_PHY_CONF0, HDMI_PHY_CONF0_GEN2_TXPWRON_OFFSET, 1);
}

static void hdmi_phy_gen2_pddq(unsigned char enable)
{
	writebf(enable, HDMI_PHY_CONF0, HDMI_PHY_CONF0_GEN2_PDDQ_OFFSET, 1);
}

/*!
 * polling the i2c operation status bit to check if the i2c r/w
 * is done before the time run out.
 *
 * @param 	msec:	time out value, using epit as the timer
 */
static int hdmi_phy_wait_i2c_done(int msec)
{
	unsigned char val = 0;

	val = readb(HDMI_IH_I2CMPHY_STAT0) & 0x3;
	while (val == 0) {
		udelay(1000);
		if (msec-- == 0) {
			printf("HDMI PHY i2c operation time out!!\n");
			return FALSE;
		}
		val = readb(HDMI_IH_I2CMPHY_STAT0) & 0x3;
	}
	return TRUE;
}

/*!
 * HDMI phy registers access through internal I2C bus
 *
 * @param	data: 	value to be send
 * @param	addr:	sub-address of registers
 *
 * @retval	TRUE:	I2C write succeed
 * @retval	FALSE:	I2C write failed/timeout  
 */
static int hdmi_phy_i2c_write(unsigned short data, unsigned char addr)
{
	writeb(0xFF, HDMI_IH_I2CMPHY_STAT0);
	writeb(addr, HDMI_PHY_I2CM_ADDRESS_ADDR);
	writeb((unsigned char)(data >> 8), HDMI_PHY_I2CM_DATAO_1_ADDR);
	writeb((unsigned char)(data >> 0), HDMI_PHY_I2CM_DATAO_0_ADDR);
	writebf(1, HDMI_PHY_I2CM_OPERATION_ADDR, 4, 1);
	return hdmi_phy_wait_i2c_done(1000);
}

/*!
 * HDMI phy registers read through internal I2C bus
 *
 * @param	addr:	sub-address of register
 *
 * @return	value read back
 */
static unsigned short hdmi_phy_i2c_read(unsigned char addr)
{
	unsigned short data;
	unsigned char msb = 0, lsb = 0;
	writeb(0xFF, HDMI_IH_I2CMPHY_STAT0);
	writeb(addr, HDMI_PHY_I2CM_ADDRESS_ADDR);
	writebf(1, HDMI_PHY_I2CM_OPERATION_ADDR, 0, 1);
	hdmi_phy_wait_i2c_done(1000);
	msb = readb(HDMI_PHY_I2CM_DATAI_1_ADDR);
	lsb = readb(HDMI_PHY_I2CM_DATAI_0_ADDR);
	data = (msb << 8) | lsb;
	return data;
}

/*!
 * HDMI phy configuration
 *
 * @param	hdmi: 	hdmi struct
 * @param	pRep:	repetition setting
 * @param	cRes: 	color resolution per component
 * @param	cscOn:	csc on/off switch
 *
 * @retval	FALSE	configuration failed
 * @retval	TRUE	configuration succeed
 */
static int hdmi_phy_configure(hdmi_data_info_s * hdmi, unsigned char pRep, unsigned char cRes, int cscOn)
{
	u8 val;
	u8 msec;

	/* color resolution 0 is 8 bit colour depth */
	if (cRes == 0)
		cRes = 8;

	if (pRep != 0) {
		printf("pixel repetition not supported %d", pRep);
		return FALSE;
	} else if (cRes != 8 && cRes != 12) {
		printf("color resolution not supported %d", cRes);
		return FALSE;
	}

	/* Enable csc path */
	if (cscOn)
		val = HDMI_MC_FLOWCTRL_FEED_THROUGH_OFF_CSC_IN_PATH;
	else
		val = HDMI_MC_FLOWCTRL_FEED_THROUGH_OFF_CSC_BYPASS;

	writeb(val, HDMI_MC_FLOWCTRL);

	/* gen2 tx power off */
	hdmi_phy_gen2_txpwron(0);

	/* gen2 pddq */
	hdmi_phy_gen2_pddq(1);

	/* PHY reset */
	writeb(HDMI_MC_PHYRSTZ_DEASSERT, HDMI_MC_PHYRSTZ);
	writeb(HDMI_MC_PHYRSTZ_ASSERT, HDMI_MC_PHYRSTZ);

	writeb(HDMI_MC_HEACPHY_RST_ASSERT, HDMI_MC_HEACPHY_RST);

	hdmi_phy_test_clear(1);
	writeb(HDMI_PHY_I2CM_SLAVE_ADDR_PHY_GEN2,
			HDMI_PHY_I2CM_SLAVE_ADDR);
	hdmi_phy_test_clear(0);

	if (hdmi->video_mode.mPixelClock <= 45250000) {
		switch (cRes) {
		case 8:
			/* PLL/MPLL Cfg */
			hdmi_phy_i2c_write(0x01e0, 0x06);
			hdmi_phy_i2c_write(0x0000, 0x15);  /* GMPCTRL */
			break;
		case 10:
			hdmi_phy_i2c_write(0x21e1, 0x06);
			hdmi_phy_i2c_write(0x0000, 0x15);
			break;
		case 12:
			hdmi_phy_i2c_write(0x41e2, 0x06);
			hdmi_phy_i2c_write(0x0000, 0x15);
			break;
		default:
			return FALSE;
		}
	} else if (hdmi->video_mode.mPixelClock <= 92500000) {
		switch (cRes) {
		case 8:
			hdmi_phy_i2c_write(0x0140, 0x06);
			hdmi_phy_i2c_write(0x0005, 0x15);
			break;
		case 10:
			hdmi_phy_i2c_write(0x2141, 0x06);
			hdmi_phy_i2c_write(0x0005, 0x15);
			break;
		case 12:
			hdmi_phy_i2c_write(0x4142, 0x06);
			hdmi_phy_i2c_write(0x0005, 0x15);
		default:
			return FALSE;
		}
	} else if (hdmi->video_mode.mPixelClock <= 148500000) {
		switch (cRes) {
		case 8:
			hdmi_phy_i2c_write(0x00a0, 0x06);
			hdmi_phy_i2c_write(0x000a, 0x15);
			break;
		case 10:
			hdmi_phy_i2c_write(0x20a1, 0x06);
			hdmi_phy_i2c_write(0x000a, 0x15);
			break;
		case 12:
			hdmi_phy_i2c_write(0x40a2, 0x06);
			hdmi_phy_i2c_write(0x000a, 0x15);
		default:
			return FALSE;
		}
	} else {
		switch (cRes) {
		case 8:
			hdmi_phy_i2c_write(0x00a0, 0x06);
			hdmi_phy_i2c_write(0x000a, 0x15);
			break;
		case 10:
			hdmi_phy_i2c_write(0x2001, 0x06);
			hdmi_phy_i2c_write(0x000f, 0x15);
			break;
		case 12:
			hdmi_phy_i2c_write(0x4002, 0x06);
			hdmi_phy_i2c_write(0x000f, 0x15);
		default:
			return FALSE;
		}
	}

	if (hdmi->video_mode.mPixelClock <= 54000000) {
		switch (cRes) {
		case 8:
			hdmi_phy_i2c_write(0x091c, 0x10);  /* CURRCTRL */
			break;
		case 10:
			hdmi_phy_i2c_write(0x091c, 0x10);
			break;
		case 12:
			hdmi_phy_i2c_write(0x06dc, 0x10);
			break;
		default:
			return FALSE;
		}
	} else if (hdmi->video_mode.mPixelClock <= 58400000) {
		switch (cRes) {
		case 8:
			hdmi_phy_i2c_write(0x091c, 0x10);
			break;
		case 10:
			hdmi_phy_i2c_write(0x06dc, 0x10);
			break;
		case 12:
			hdmi_phy_i2c_write(0x06dc, 0x10);
			break;
		default:
			return FALSE;
		}
	} else if (hdmi->video_mode.mPixelClock <= 72000000) {
		switch (cRes) {
		case 8:
			hdmi_phy_i2c_write(0x06dc, 0x10);
			break;
		case 10:
			hdmi_phy_i2c_write(0x06dc, 0x10);
			break;
		case 12:
			hdmi_phy_i2c_write(0x091c, 0x10);
			break;
		default:
			return FALSE;
		}
	} else if (hdmi->video_mode.mPixelClock <= 74250000) {
		switch (cRes) {
		case 8:
			hdmi_phy_i2c_write(0x06dc, 0x10);
			break;
		case 10:
			hdmi_phy_i2c_write(0x0b5c, 0x10);
			break;
		case 12:
			hdmi_phy_i2c_write(0x091c, 0x10);
			break;
		default:
			return FALSE;
		}
	} else if (hdmi->video_mode.mPixelClock <= 118800000) {
		switch (cRes) {
		case 8:
			hdmi_phy_i2c_write(0x091c, 0x10);
			break;
		case 10:
			hdmi_phy_i2c_write(0x091c, 0x10);
			break;
		case 12:
			hdmi_phy_i2c_write(0x06dc, 0x10);
			break;
		default:
			return FALSE;
		}
	} else if (hdmi->video_mode.mPixelClock <= 216000000) {
		switch (cRes) {
		case 8:
			hdmi_phy_i2c_write(0x06dc, 0x10);
			break;
		case 10:
			hdmi_phy_i2c_write(0x0b5c, 0x10);
			break;
		case 12:
			hdmi_phy_i2c_write(0x091c, 0x10);
			break;
		default:
			return FALSE;
		}
	} else {
		printf("Pixel clock %d - unsupported by HDMI\n",
				hdmi->video_mode.mPixelClock);
		return FALSE;
	}

	hdmi_phy_i2c_write(0x0000, 0x13);  /* PLLPHBYCTRL */
	hdmi_phy_i2c_write(0x0006, 0x17);
	/* RESISTANCE TERM 133Ohm Cfg */
	hdmi_phy_i2c_write(0x0005, 0x19);  /* TXTERM */
	/* PREEMP Cgf 0.00 */
	hdmi_phy_i2c_write(0x800d, 0x09);  /* CKSYMTXCTRL */
	/* TX/CK LVL 10 */
	hdmi_phy_i2c_write(0x01ad, 0x0E);  /* VLEVCTRL */
	/* REMOVE CLK TERM */
	hdmi_phy_i2c_write(0x8000, 0x05);  /* CKCALCTRL */

	if (hdmi->video_mode.mPixelClock > 148500000) {
			hdmi_phy_i2c_write(0x800b, 0x09);
			hdmi_phy_i2c_write(0x0129, 0x0E);
	}

	hdmi_phy_enable_power(1);

	/* toggle TMDS enable */
	hdmi_phy_enable_tmds(0);
	hdmi_phy_enable_tmds(1);

	/* gen2 tx power on */
	hdmi_phy_gen2_txpwron(1);
	hdmi_phy_gen2_pddq(0);

	/*Wait for PHY PLL lock */
	msec = 4;
	val = readb(HDMI_PHY_STAT0) & HDMI_PHY_TX_PHY_LOCK;
	while (val == 0) {
		udelay(1000);
		if (msec-- == 0) {
			printf("PHY PLL not locked\n");
			return FALSE;
		}
		val = readb(HDMI_PHY_STAT0) & HDMI_PHY_TX_PHY_LOCK;
	}

	return TRUE;
}

/*!
 * HDMI phy initialization
 */
static void hdmi_phy_init(hdmi_data_info_s * hdmi)
{
	int i;
	int cscon = FALSE;

	/*check csc whether needed activated in HDMI mode */
	cscon = (isColorSpaceConversion(hdmi) &&
			!hdmi->video_mode.mDVI);

	/* HDMI Phy spec says to do the phy initialization sequence twice */
	for (i = 0 ; i < 2 ; i++) {
		hdmi_phy_int_mask(0xFF);
		hdmi_phy_sel_data_en_pol((hdmi->video_mode.mDataEnablePolarity == TRUE) ? 1 : 0);
		hdmi_phy_sel_interface_control(0);
		hdmi_phy_enable_tmds(0);
		hdmi_phy_enable_power(0);

		hdmi_phy_configure(hdmi, 0, 8, cscon);
	}
}

/*!
 * mute hdmi audio output
 */
static void hdmi_audio_mute(hdmi_data_info_s * hdmi, int mute)
{
	writebf((mute == TRUE) ? 0xF : 0, HDMI_FC_AUDSCONF, 4, 4);
}

static void hdmi_enable_audio_clk(hdmi_data_info_s * hdmi, int enable)
{
	u8 clkdis;

	clkdis = readb(HDMI_MC_CLKDIS);
	if (enable)
		clkdis &= ~HDMI_MC_CLKDIS_AUDCLK_DISABLE;
	else
		clkdis |= HDMI_MC_CLKDIS_AUDCLK_DISABLE;
	writeb(clkdis, HDMI_MC_CLKDIS);
}

/*!
 * this submodule is responsible for the video/audio data composition.
 * video mode is set here, but the actual flow is determined by the input.
 */
static void hdmi_av_composer(hdmi_data_info_s * hdmi)
{
	u8 inv_val;

	/* Set up HDMI_FC_INVIDCONF */
	inv_val = (hdmi->hdcp_enable ?
		HDMI_FC_INVIDCONF_HDCP_KEEPOUT_ACTIVE :
		HDMI_FC_INVIDCONF_HDCP_KEEPOUT_INACTIVE);

	inv_val |= (hdmi->video_mode.mVSyncPolarity ?
		HDMI_FC_INVIDCONF_VSYNC_IN_POLARITY_ACTIVE_HIGH :
		HDMI_FC_INVIDCONF_VSYNC_IN_POLARITY_ACTIVE_LOW);

	inv_val |= (hdmi->video_mode.mHSyncPolarity ?
		HDMI_FC_INVIDCONF_HSYNC_IN_POLARITY_ACTIVE_HIGH :
		HDMI_FC_INVIDCONF_HSYNC_IN_POLARITY_ACTIVE_LOW);

	inv_val |= (hdmi->video_mode.mDataEnablePolarity ?
		HDMI_FC_INVIDCONF_DE_IN_POLARITY_ACTIVE_HIGH :
		HDMI_FC_INVIDCONF_DE_IN_POLARITY_ACTIVE_LOW);

	if (hdmi->video_mode.mCode == 39)
		inv_val |= HDMI_FC_INVIDCONF_R_V_BLANK_IN_OSC_ACTIVE_HIGH;
	else
		inv_val |= (hdmi->video_mode.mInterlaced ?
			HDMI_FC_INVIDCONF_R_V_BLANK_IN_OSC_ACTIVE_HIGH :
			HDMI_FC_INVIDCONF_R_V_BLANK_IN_OSC_ACTIVE_LOW);

	inv_val |= (hdmi->video_mode.mInterlaced ?
		HDMI_FC_INVIDCONF_IN_I_P_INTERLACED :
		HDMI_FC_INVIDCONF_IN_I_P_PROGRESSIVE);

	inv_val |= (hdmi->video_mode.mDVI ?
		HDMI_FC_INVIDCONF_DVI_MODEZ_DVI_MODE :
		HDMI_FC_INVIDCONF_DVI_MODEZ_HDMI_MODE);

	writeb(inv_val, HDMI_FC_INVIDCONF);

	/* Set up horizontal active pixel region width */
	writeb(hdmi->video_mode.mHActive >> 8, HDMI_FC_INHACTV1);
	writeb(hdmi->video_mode.mHActive, HDMI_FC_INHACTV0);

	/* Set up vertical blanking pixel region width */
	writeb(hdmi->video_mode.mVActive >> 8, HDMI_FC_INVACTV1);
	writeb(hdmi->video_mode.mVActive, HDMI_FC_INVACTV0);

	/* Set up horizontal blanking pixel region width */
	writeb(hdmi->video_mode.mHBlanking >> 8, HDMI_FC_INHBLANK1);
	writeb(hdmi->video_mode.mHBlanking, HDMI_FC_INHBLANK0);

	/* Set up vertical blanking pixel region width */
	writeb(hdmi->video_mode.mVBlanking, HDMI_FC_INVBLANK);

	/* Set up HSYNC active edge delay width (in pixel clks) */
	writeb(hdmi->video_mode.mHSyncOffset >> 8, HDMI_FC_HSYNCINDELAY1);
	writeb(hdmi->video_mode.mHSyncOffset, HDMI_FC_HSYNCINDELAY0);

	/* Set up VSYNC active edge delay (in pixel clks) */
	writeb(hdmi->video_mode.mVSyncOffset, HDMI_FC_VSYNCINDELAY);

	/* Set up HSYNC active pulse width (in pixel clks) */
	writeb(hdmi->video_mode.mHSyncPulseWidth >> 8, HDMI_FC_HSYNCINWIDTH1);
	writeb(hdmi->video_mode.mHSyncPulseWidth, HDMI_FC_HSYNCINWIDTH0);

	/* Set up VSYNC active edge delay (in pixel clks) */
	writeb(hdmi->video_mode.mVSyncPulseWidth, HDMI_FC_VSYNCINWIDTH);
}

static void hdmi_config_AVI(hdmi_data_info_s * hdmi)
{
	u8 val;
	u8 pix_fmt;
	u8 under_scan;
	u8 act_ratio, coded_ratio, colorimetry, ext_colorimetry;
	int aspect_16_9;

	if ((hdmi->video_mode.mHImageSize == 4) && (hdmi->video_mode.mVImageSize == 3))
		aspect_16_9 = FALSE;
	else
		aspect_16_9 = TRUE;

	/********************************************
	 * AVI Data Byte 1
	 ********************************************/
	if (hdmi->enc_out_format == eYCC444)
		pix_fmt = HDMI_FC_AVICONF0_PIX_FMT_YCBCR444;
	else if (hdmi->enc_out_format == eYCC422)
		pix_fmt = HDMI_FC_AVICONF0_PIX_FMT_YCBCR422;
	else
		pix_fmt = HDMI_FC_AVICONF0_PIX_FMT_RGB;

	under_scan = HDMI_FC_AVICONF0_SCAN_INFO_NODATA;

	/*
	 * Active format identification data is present in the AVI InfoFrame.
	 * Under scan info, no bar data
	 */
	val = pix_fmt | under_scan |
		HDMI_FC_AVICONF0_ACTIVE_FMT_INFO_PRESENT |
		HDMI_FC_AVICONF0_BAR_DATA_NO_DATA;

	writeb(val, HDMI_FC_AVICONF0);

	/********************************************
	 * AVI Data Byte 2
	 ********************************************/

	/*	Set the Aspect Ratio */
	if (aspect_16_9) {
		act_ratio = HDMI_FC_AVICONF1_ACTIVE_ASPECT_RATIO_16_9;
		coded_ratio = HDMI_FC_AVICONF1_CODED_ASPECT_RATIO_16_9;
	} else {
		act_ratio = HDMI_FC_AVICONF1_ACTIVE_ASPECT_RATIO_4_3;
		coded_ratio = HDMI_FC_AVICONF1_CODED_ASPECT_RATIO_4_3;
	}

	/* Set up colorimetry */
	if (hdmi->enc_out_format == eXVYCC444) {
		colorimetry = HDMI_FC_AVICONF1_COLORIMETRY_EXTENDED_INFO;
		if (hdmi->colorimetry == eITU601)
			ext_colorimetry =
				HDMI_FC_AVICONF2_EXT_COLORIMETRY_XVYCC601;
		else /* hdmi->colorimetry == eITU709 */
			ext_colorimetry =
				HDMI_FC_AVICONF2_EXT_COLORIMETRY_XVYCC709;
	} else if (hdmi->enc_out_format != eRGB) {
		if (hdmi->colorimetry == eITU601)
			colorimetry = HDMI_FC_AVICONF1_COLORIMETRY_SMPTE;
		else /* hdmi->colorimetry == eITU709 */
			colorimetry = HDMI_FC_AVICONF1_COLORIMETRY_ITUR;
		ext_colorimetry = HDMI_FC_AVICONF2_EXT_COLORIMETRY_XVYCC601;
	} else { /* Carries no data */
		colorimetry = HDMI_FC_AVICONF1_COLORIMETRY_NO_DATA;
		ext_colorimetry = HDMI_FC_AVICONF2_EXT_COLORIMETRY_XVYCC601;
	}

	val = colorimetry | coded_ratio | act_ratio;
	writeb(val, HDMI_FC_AVICONF1);

	/********************************************
	 * AVI Data Byte 3
	 ********************************************/

	val = HDMI_FC_AVICONF2_IT_CONTENT_NO_DATA | ext_colorimetry |
		HDMI_FC_AVICONF2_RGB_QUANT_DEFAULT |
		HDMI_FC_AVICONF2_SCALING_NONE;
	writeb(val, HDMI_FC_AVICONF2);

	/********************************************
	 * AVI Data Byte 4
	 ********************************************/
	writeb(hdmi->video_mode.mCode, HDMI_FC_AVIVID);

	/********************************************
	 * AVI Data Byte 5
	 ********************************************/

	/* Set up input and output pixel repetition */
	val = (((hdmi->video_mode.mPixelRepetitionInput  + 1) <<
		HDMI_FC_PRCONF_INCOMING_PR_FACTOR_OFFSET) &
		HDMI_FC_PRCONF_INCOMING_PR_FACTOR_MASK) |
		((hdmi->video_mode.mPixelRepetitionOutput <<
		HDMI_FC_PRCONF_OUTPUT_PR_FACTOR_OFFSET) &
		HDMI_FC_PRCONF_OUTPUT_PR_FACTOR_MASK);
	writeb(val, HDMI_FC_PRCONF);

	/* IT Content and quantization range = don't care */
	val = HDMI_FC_AVICONF3_IT_CONTENT_TYPE_GRAPHICS |
		HDMI_FC_AVICONF3_QUANT_RANGE_LIMITED;
	writeb(val, HDMI_FC_AVICONF3);

	/********************************************
	 * AVI Data Bytes 6-13
	 ********************************************/
	writeb(0, HDMI_FC_AVIETB0);
	writeb(0, HDMI_FC_AVIETB1);
	writeb(0, HDMI_FC_AVISBB0);
	writeb(0, HDMI_FC_AVISBB1);
	writeb(0, HDMI_FC_AVIELB0);
	writeb(0, HDMI_FC_AVIELB1);
	writeb(0, HDMI_FC_AVISRB0);
	writeb(0, HDMI_FC_AVISRB1);
}

static void hdmi_enable_video_path(hdmi_data_info_s * hdmi)
{
	u8 clkdis;

	/* control period minimum duration */
	writeb(12, HDMI_FC_CTRLDUR);
	writeb(32, HDMI_FC_EXCTRLDUR);
	writeb(1, HDMI_FC_EXCTRLSPAC);

	/* Set to fill TMDS data channels */
	writeb(0x0B, HDMI_FC_CH0PREAM);
	writeb(0x16, HDMI_FC_CH1PREAM);
	writeb(0x21, HDMI_FC_CH2PREAM);

	/* Enable pixel clock and tmds data path */
	clkdis = 0x7F;
	clkdis &= ~HDMI_MC_CLKDIS_PIXELCLK_DISABLE;
	writeb(clkdis, HDMI_MC_CLKDIS);

	clkdis &= ~HDMI_MC_CLKDIS_TMDSCLK_DISABLE;
	writeb(clkdis, HDMI_MC_CLKDIS);

	/* Enable csc path */
	if (isColorSpaceConversion(hdmi)) {
		clkdis &= ~HDMI_MC_CLKDIS_CSCCLK_DISABLE;
		writeb(clkdis, HDMI_MC_CLKDIS);
	}
}

/*!
 * HDCP configuration, disabled here
 */
static void hdmi_tx_hdcp_config(hdmi_data_info_s * hdmi)
{
	u8 de, val;

	if (hdmi->video_mode.mDataEnablePolarity)
		de = HDMI_A_VIDPOLCFG_DATAENPOL_ACTIVE_HIGH;
	else
		de = HDMI_A_VIDPOLCFG_DATAENPOL_ACTIVE_LOW;

	/* disable rx detect */
	val = readb(HDMI_A_HDCPCFG0);
	val &= HDMI_A_HDCPCFG0_RXDETECT_MASK;
	val |= HDMI_A_HDCPCFG0_RXDETECT_DISABLE;
	writeb(val, HDMI_A_HDCPCFG0);

	val = readb(HDMI_A_VIDPOLCFG);
	val &= HDMI_A_VIDPOLCFG_DATAENPOL_MASK;
	val |= de;
	writeb(val, HDMI_A_VIDPOLCFG);

	val = readb(HDMI_A_HDCPCFG1);
	val &= HDMI_A_HDCPCFG1_ENCRYPTIONDISABLE_MASK;
	val |= HDMI_A_HDCPCFG1_ENCRYPTIONDISABLE_DISABLE;
	writeb(val, HDMI_A_HDCPCFG1);
}

/*!
 * this submodule is responsible for the video data synchronization.
 * for example, for RGB 4:4:4 input, the data map is defined as
 * 			pin{47~40} <==> R[7:0]
 * 			pin{31~24} <==> G[7:0]
 * 			pin{15~8}  <==> B[7:0]
 */
static void hdmi_video_sample(hdmi_data_info_s * hdmi)
{
	int color_format = 0;
	u8 val;

	if (hdmi->enc_in_format == eRGB) {
		if (hdmi->enc_color_depth == 8) {
			color_format = 0x01;
		} else if (hdmi->enc_color_depth == 10) {
			color_format = 0x03;
		} else if (hdmi->enc_color_depth == 12) {
			color_format = 0x05;
		} else if (hdmi->enc_color_depth == 16) {
			color_format = 0x07;
		} else {
			printf("Invalid color depth %d!!", hdmi->enc_color_depth);
			return;
		}
	} else if (hdmi->enc_in_format == eYCC444) {
		if (hdmi->enc_color_depth == 8) {
			color_format = 0x09;
		} else if (hdmi->enc_color_depth == 10) {
			color_format = 0x0B;
		} else if (hdmi->enc_color_depth == 12) {
			color_format = 0x0D;
		} else if (hdmi->enc_color_depth == 16) {
			color_format = 0x0F;
		} else {
			printf("Invalid color depth %d!!", hdmi->enc_color_depth);
			return;
		}
	} else if (hdmi->enc_in_format == eYCC422_8bits) {
		if (hdmi->enc_color_depth == 8) {
			color_format = 0x16;
		} else if (hdmi->enc_color_depth == 10) {
			color_format = 0x14;
		} else if (hdmi->enc_color_depth == 12) {
			color_format = 0x12;
		} else {
			printf("Invalid color depth %d!!", hdmi->enc_color_depth);
			return;
		}
	}

	val = HDMI_TX_INVID0_INTERNAL_DE_GENERATOR_DISABLE |
		((color_format << HDMI_TX_INVID0_VIDEO_MAPPING_OFFSET) &
		HDMI_TX_INVID0_VIDEO_MAPPING_MASK);
	writeb(val, HDMI_TX_INVID0);

	/* Enable TX stuffing: When DE is inactive, fix the output data to 0 */
	val = HDMI_TX_INSTUFFING_BDBDATA_STUFFING_ENABLE |
		HDMI_TX_INSTUFFING_RCRDATA_STUFFING_ENABLE |
		HDMI_TX_INSTUFFING_GYDATA_STUFFING_ENABLE;
	writeb(val, HDMI_TX_INSTUFFING);
	writeb(0x0, HDMI_TX_GYDATA0);
	writeb(0x0, HDMI_TX_GYDATA1);
	writeb(0x0, HDMI_TX_RCRDATA0);
	writeb(0x0, HDMI_TX_RCRDATA1);
	writeb(0x0, HDMI_TX_BCBDATA0);
	writeb(0x0, HDMI_TX_BCBDATA1);
}

/*!
 * set HDMI color space conversion module.
 */
static void hdmi_video_csc(hdmi_data_info_s * hdmi)
{
	int color_depth = 0;
	int interpolation = HDMI_CSC_CFG_INTMODE_DISABLE;
	int decimation = 0;
	u8 val;

	/*YCC422 interpolation to 444 mode */
	if (isColorSpaceInterpolation(hdmi) == TRUE) {
		interpolation = HDMI_CSC_CFG_INTMODE_CHROMA_INT_FORMULA1;
	} else if (isColorSpaceDecimation(hdmi) == TRUE) {
		decimation = HDMI_CSC_CFG_DECMODE_CHROMA_INT_FORMULA3;
	}

	if (hdmi->enc_color_depth == 8) {
		color_depth = HDMI_CSC_SCALE_CSC_COLORDE_PTH_24BPP;
	} else if (hdmi->enc_color_depth == 10) {
		color_depth = HDMI_CSC_SCALE_CSC_COLORDE_PTH_30BPP;
	} else if (hdmi->enc_color_depth == 12) {
		color_depth = HDMI_CSC_SCALE_CSC_COLORDE_PTH_36BPP;
	} else if (hdmi->enc_color_depth == 16) {
		color_depth = HDMI_CSC_SCALE_CSC_COLORDE_PTH_48BPP;
	} else {
		printf("Invalid color depth %d!!", hdmi->enc_color_depth);
		return;
	}

	/*configure the CSC registers */
	writeb(interpolation | decimation, HDMI_CSC_CFG);
	val = readb(HDMI_CSC_SCALE);
	val &= ~HDMI_CSC_SCALE_CSC_COLORDE_PTH_MASK;
	val |= color_depth;
	writeb(val, HDMI_CSC_SCALE);

	update_csc_coeffs(hdmi);
}

/*!
 * HDMI video packetizer is used to packetize the data.
 * for example, if input is YCC422 mode or repeater is used, data should be repacked
 * this module can be bypassed.
 */
static void hdmi_video_packetize(hdmi_data_info_s * hdmi)
{
	unsigned int color_depth = 0;
	unsigned int remap_size = HDMI_VP_REMAP_YCC422_16bit;
	unsigned int output_select = HDMI_VP_CONF_OUTPUT_SELECTOR_PP;
	u8 val;

	if (hdmi->enc_out_format == eRGB || hdmi->enc_out_format == eYCC444) {
		if (hdmi->enc_color_depth == 0) {
			output_select = HDMI_VP_CONF_OUTPUT_SELECTOR_BYPASS;
		} else if (hdmi->enc_color_depth == 8) {
			color_depth = 4;
			output_select = HDMI_VP_CONF_OUTPUT_SELECTOR_BYPASS;
		} else if (hdmi->enc_color_depth == 10) {
			color_depth = 5;
		} else if (hdmi->enc_color_depth == 12) {
			color_depth = 6;
		} else if (hdmi->enc_color_depth == 16) {
			color_depth = 7;
		} else {
			printf("Invalid color depth %d!!\n", hdmi->enc_color_depth);
			return;
		}
	} else if (hdmi->enc_out_format == eYCC422) {
		if (hdmi->enc_color_depth == 0 || hdmi->enc_color_depth == 8) {
			remap_size = HDMI_VP_REMAP_YCC422_16bit;
		} else if (hdmi->enc_color_depth == 10) {
			remap_size = HDMI_VP_REMAP_YCC422_20bit;
		} else if (hdmi->enc_color_depth == 12) {
			remap_size = HDMI_VP_REMAP_YCC422_24bit;
		} else {
			printf("Invalid color remap size %d!!\n", hdmi->enc_color_depth);
			return;
		}
		output_select = HDMI_VP_CONF_OUTPUT_SELECTOR_YCC422;
	} else {
		printf("Invalid output encoding type %d!!\n", hdmi->enc_out_format);
		return;
	}

	/* HDMI not support deep color,
	 * because IPU MAX support color depth is 24bit */
	color_depth = 0;

	/* set the packetizer registers */
	val = ((color_depth << HDMI_VP_PR_CD_COLOR_DEPTH_OFFSET) &
		HDMI_VP_PR_CD_COLOR_DEPTH_MASK) |
		((hdmi->pix_repet_factor <<
		HDMI_VP_PR_CD_DESIRED_PR_FACTOR_OFFSET) &
		HDMI_VP_PR_CD_DESIRED_PR_FACTOR_MASK);
	writeb(val, HDMI_VP_PR_CD);

	val = readb(HDMI_VP_STUFF);
	val &= ~HDMI_VP_STUFF_PR_STUFFING_MASK;
	val |= HDMI_VP_STUFF_PR_STUFFING_STUFFING_MODE;
	writeb(val, HDMI_VP_STUFF);

	/* Data from pixel repeater block */
	if (hdmi->pix_repet_factor > 1) {
		val = readb(HDMI_VP_CONF);
		val &= ~(HDMI_VP_CONF_PR_EN_MASK |
			HDMI_VP_CONF_BYPASS_SELECT_MASK);
		val |= HDMI_VP_CONF_PR_EN_ENABLE |
			HDMI_VP_CONF_BYPASS_SELECT_PIX_REPEATER;
		writeb(val, HDMI_VP_CONF);
	} else { /* data from packetizer block */
		val = readb(HDMI_VP_CONF);
		val &= ~(HDMI_VP_CONF_PR_EN_MASK |
			HDMI_VP_CONF_BYPASS_SELECT_MASK);
		val |= HDMI_VP_CONF_PR_EN_DISABLE |
			HDMI_VP_CONF_BYPASS_SELECT_VID_PACKETIZER;
		writeb(val, HDMI_VP_CONF);
	}

	val = readb(HDMI_VP_STUFF);
	val &= ~HDMI_VP_STUFF_IDEFAULT_PHASE_MASK;
	val |= 1 << HDMI_VP_STUFF_IDEFAULT_PHASE_OFFSET;
	writeb(val, HDMI_VP_STUFF);

	writeb(remap_size, HDMI_VP_REMAP);

	if (output_select == HDMI_VP_CONF_OUTPUT_SELECTOR_PP) {
		val = readb(HDMI_VP_CONF);
		val &= ~(HDMI_VP_CONF_BYPASS_EN_MASK |
			HDMI_VP_CONF_PP_EN_ENMASK |
			HDMI_VP_CONF_YCC422_EN_MASK);
		val |= HDMI_VP_CONF_BYPASS_EN_DISABLE |
			HDMI_VP_CONF_PP_EN_ENABLE |
			HDMI_VP_CONF_YCC422_EN_DISABLE;
		writeb(val, HDMI_VP_CONF);
	} else if (output_select == HDMI_VP_CONF_OUTPUT_SELECTOR_YCC422) {
		val = readb(HDMI_VP_CONF);
		val &= ~(HDMI_VP_CONF_BYPASS_EN_MASK |
			HDMI_VP_CONF_PP_EN_ENMASK |
			HDMI_VP_CONF_YCC422_EN_MASK);
		val |= HDMI_VP_CONF_BYPASS_EN_DISABLE |
			HDMI_VP_CONF_PP_EN_DISABLE |
			HDMI_VP_CONF_YCC422_EN_ENABLE;
		writeb(val, HDMI_VP_CONF);
	} else if (output_select == HDMI_VP_CONF_OUTPUT_SELECTOR_BYPASS) {
		val = readb(HDMI_VP_CONF);
		val &= ~(HDMI_VP_CONF_BYPASS_EN_MASK |
			HDMI_VP_CONF_PP_EN_ENMASK |
			HDMI_VP_CONF_YCC422_EN_MASK);
		val |= HDMI_VP_CONF_BYPASS_EN_ENABLE |
			HDMI_VP_CONF_PP_EN_DISABLE |
			HDMI_VP_CONF_YCC422_EN_DISABLE;
		writeb(val, HDMI_VP_CONF);
	} else {
		return;
	}

	val = readb(HDMI_VP_STUFF);
	val &= ~(HDMI_VP_STUFF_PP_STUFFING_MASK |
		HDMI_VP_STUFF_YCC422_STUFFING_MASK);
	val |= HDMI_VP_STUFF_PP_STUFFING_STUFFING_MODE |
		HDMI_VP_STUFF_YCC422_STUFFING_STUFFING_MODE;
	writeb(val, HDMI_VP_STUFF);

	val = readb(HDMI_VP_CONF);
	val &= ~HDMI_VP_CONF_OUTPUT_SELECTOR_MASK;
	val |= output_select;
	writeb(val, HDMI_VP_CONF);
}

/*!
 * this submodule is responsible for the video/audio data composition.
 */
static void hdmi_set_video_mode(hdmi_data_info_s * hdmi)
{
	hdmi_vmode_s * vmode = &(hdmi->video_mode);

	vmode->mHBorder = 0;
	vmode->mVBorder = 0;
	vmode->mPixelRepetitionInput = 0;
	vmode->mHImageSize = 16;
	vmode->mVImageSize = 9;

	if ((vmode->mCode == 6) || (vmode->mCode == 7) ||
		(vmode->mCode == 21) || (vmode->mCode == 22) ||
		(vmode->mCode == 2) || (vmode->mCode == 3) ||
		(vmode->mCode == 17) || (vmode->mCode == 18))
		hdmi->colorimetry = eITU601;
	else
		hdmi->colorimetry = eITU709;

	if ((vmode->mCode == 10) || (vmode->mCode == 11) ||
		(vmode->mCode == 12) || (vmode->mCode == 13) ||
		(vmode->mCode == 14) || (vmode->mCode == 15) ||
		(vmode->mCode == 25) || (vmode->mCode == 26) ||
		(vmode->mCode == 27) || (vmode->mCode == 28) ||
		(vmode->mCode == 29) || (vmode->mCode == 30) ||
		(vmode->mCode == 35) || (vmode->mCode == 36) ||
		(vmode->mCode == 37) || (vmode->mCode == 38))
		vmode->mPixelRepetitionOutput = 1;
	else
		vmode->mPixelRepetitionOutput = 0;

	switch (vmode->mCode) {
		case 1:                    //640x480p @ 59.94/60Hz 4:3
			vmode->mHImageSize = 4;
			vmode->mVImageSize = 3;
			vmode->mHActive = 640;
			vmode->mVActive = 480;
			vmode->mHBlanking = 160;
			vmode->mVBlanking = 45;
			vmode->mHSyncOffset = 16;
			vmode->mVSyncOffset = 10;
			vmode->mHSyncPulseWidth = 96;
			vmode->mVSyncPulseWidth = 2;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = FALSE;
			vmode->mInterlaced = FALSE; // not(progressive_nI)
			vmode->mPixelClock = (vmode->mRefreshRate == 59940) ? 25170000 : 25200000;
			break;
		case 2:                    //720x480p @ 59.94/60Hz 4:3
			vmode->mHImageSize = 4;
			vmode->mVImageSize = 3;
		case 3:                    //720x480p @ 59.94/60Hz 16:9
			vmode->mHActive = 720;
			vmode->mVActive = 480;
			vmode->mHBlanking = 138;
			vmode->mVBlanking = 45;
			vmode->mHSyncOffset = 16;
			vmode->mVSyncOffset = 9;
			vmode->mHSyncPulseWidth = 62;
			vmode->mVSyncPulseWidth = 6;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = FALSE;
			vmode->mInterlaced = FALSE;
			vmode->mPixelClock = (vmode->mRefreshRate == 59940) ? 27000000 : 27020000;
			break;
		case 4:                    //1280x720p @ 59.94/60Hz 16:9
			vmode->mHActive = 1280;
			vmode->mVActive = 720;
			vmode->mHBlanking = 370;
			vmode->mVBlanking = 30;
			vmode->mHSyncOffset = 110;
			vmode->mVSyncOffset = 5;
			vmode->mHSyncPulseWidth = 40;
			vmode->mVSyncPulseWidth = 5;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = TRUE;
			vmode->mInterlaced = FALSE;
			vmode->mPixelClock = (vmode->mRefreshRate == 59940) ? 74170000 : 74250000;
			break;
		case 5:                    //1920x1080i @ 59.94/60Hz 16:9
			vmode->mHActive = 1920;
			vmode->mVActive = 540;
			vmode->mHBlanking = 280;
			vmode->mVBlanking = 22;
			vmode->mHSyncOffset = 88;
			vmode->mVSyncOffset = 2;
			vmode->mHSyncPulseWidth = 44;
			vmode->mVSyncPulseWidth = 5;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = TRUE;
			vmode->mInterlaced = TRUE;
			vmode->mPixelClock = (vmode->mRefreshRate == 59940) ? 74170000 : 74250000;
			break;
		case 6:                    //720(1440)x480i @ 59.94/60Hz 4:3
			vmode->mHImageSize = 4;
			vmode->mVImageSize = 3;
		case 7:                    //720(1440)x480i @ 59.94/60Hz 16:9
			vmode->mHActive = 1440;
			vmode->mVActive = 240;
			vmode->mHBlanking = 276;
			vmode->mVBlanking = 22;
			vmode->mHSyncOffset = 38;
			vmode->mVSyncOffset = 4;
			vmode->mHSyncPulseWidth = 124;
			vmode->mVSyncPulseWidth = 3;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = FALSE;
			vmode->mInterlaced = TRUE;
			vmode->mPixelClock = (vmode->mRefreshRate == 59940) ? 27000000 : 27020000;
			vmode->mPixelRepetitionInput = 1;
			break;
		case 8:                    //720(1440)x240p @ 59.826/60.054/59.886/60.115Hz 4:3
			vmode->mHImageSize = 4;
			vmode->mVImageSize = 3;
		case 9:                    //720(1440)x240p @ 59.826/60.054/59.886/60.115Hz 16:9
			vmode->mHActive = 1440;
			vmode->mVActive = 240;
			vmode->mHBlanking = 276;
			vmode->mVBlanking = (vmode->mRefreshRate > 60000) ? 22 : 23;
			vmode->mHSyncOffset = 38;
			vmode->mVSyncOffset = (vmode->mRefreshRate > 60000) ? 4 : 5;
			vmode->mHSyncPulseWidth = 124;
			vmode->mVSyncPulseWidth = 3;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = FALSE;
			vmode->mInterlaced = FALSE;
			vmode->mPixelClock = ((vmode->mRefreshRate == 60054) || vmode->mRefreshRate == 59826) ? 27000000 : 27020000;    // else 60.115/59.886 Hz
			vmode->mPixelRepetitionInput = 1;
			break;
		case 10:                   //2880x480i @ 59.94/60Hz 4:3
			vmode->mHImageSize = 4;
			vmode->mVImageSize = 3;
		case 11:                   //2880x480i @ 59.94/60Hz 16:9
			vmode->mHActive = 2880;
			vmode->mVActive = 240;
			vmode->mHBlanking = 552;
			vmode->mVBlanking = 22;
			vmode->mHSyncOffset = 76;
			vmode->mVSyncOffset = 4;
			vmode->mHSyncPulseWidth = 248;
			vmode->mVSyncPulseWidth = 3;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = FALSE;
			vmode->mInterlaced = TRUE;
			vmode->mPixelClock = (vmode->mRefreshRate == 59940) ? 54000000 : 54050000;
			break;
		case 12:                   //2880x240p @ 59.826/60.054/59.886/60.115Hz 4:3
			vmode->mHImageSize = 4;
			vmode->mVImageSize = 3;
		case 13:                   //2880x240p @ 59.826/60.054/59.886/60.115Hz 16:9
			vmode->mHActive = 2880;
			vmode->mVActive = 240;
			vmode->mHBlanking = 552;
			vmode->mVBlanking = (vmode->mRefreshRate > 60000) ? 22 : 23;
			vmode->mHSyncOffset = 76;
			vmode->mVSyncOffset = (vmode->mRefreshRate > 60000) ? 4 : 5;
			vmode->mHSyncPulseWidth = 248;
			vmode->mVSyncPulseWidth = 3;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = FALSE;
			vmode->mInterlaced = FALSE;
			vmode->mPixelClock = ((vmode->mRefreshRate == 60054) || vmode->mRefreshRate == 59826) ? 54000000 : 54050000;    // else 60.115/59.886 Hz
			break;
		case 14:                   //1440x480p @ 59.94/60Hz 4:3
			vmode->mHImageSize = 4;
			vmode->mVImageSize = 3;
		case 15:                   //1440x480p @ 59.94/60Hz 16:9
			vmode->mHActive = 1440;
			vmode->mVActive = 480;
			vmode->mHBlanking = 276;
			vmode->mVBlanking = 45;
			vmode->mHSyncOffset = 32;
			vmode->mVSyncOffset = 9;
			vmode->mHSyncPulseWidth = 124;
			vmode->mVSyncPulseWidth = 6;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = FALSE;
			vmode->mInterlaced = FALSE;
			vmode->mPixelClock = (vmode->mRefreshRate == 59940) ? 54000000 : 54050000;
			break;
		case 16:                   //1920x1080p @ 59.94/60Hz 16:9
			vmode->mHActive = 1920;
			vmode->mVActive = 1080;
			vmode->mHBlanking = 280;
			vmode->mVBlanking = 45;
			vmode->mHSyncOffset = 88;
			vmode->mVSyncOffset = 4;
			vmode->mHSyncPulseWidth = 44;
			vmode->mVSyncPulseWidth = 5;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = TRUE;
			vmode->mInterlaced = FALSE;
			vmode->mPixelClock = (vmode->mRefreshRate == 59940) ? 148350000 : 148500000;
			break;
		case 17:                   //720x576p @ 50Hz 4:3
			vmode->mHImageSize = 4;
			vmode->mVImageSize = 3;
		case 18:                   //720x576p @ 50Hz 16:9
			vmode->mHActive = 720;
			vmode->mVActive = 576;
			vmode->mHBlanking = 144;
			vmode->mVBlanking = 49;
			vmode->mHSyncOffset = 12;
			vmode->mVSyncOffset = 5;
			vmode->mHSyncPulseWidth = 64;
			vmode->mVSyncPulseWidth = 5;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = FALSE;
			vmode->mInterlaced = FALSE;
			vmode->mPixelClock = 27000000;
			break;
		case 19:                   //1280x720p @ 50Hz 16:9
			vmode->mHActive = 1280;
			vmode->mVActive = 720;
			vmode->mHBlanking = 700;
			vmode->mVBlanking = 30;
			vmode->mHSyncOffset = 440;
			vmode->mVSyncOffset = 5;
			vmode->mHSyncPulseWidth = 40;
			vmode->mVSyncPulseWidth = 5;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = TRUE;
			vmode->mInterlaced = FALSE;
			vmode->mPixelClock = 74250000;
			break;
		case 20:                   //1920x1080i @ 50Hz 16:9
			vmode->mHActive = 1920;
			vmode->mVActive = 540;
			vmode->mHBlanking = 720;
			vmode->mVBlanking = 22;
			vmode->mHSyncOffset = 528;
			vmode->mVSyncOffset = 2;
			vmode->mHSyncPulseWidth = 44;
			vmode->mVSyncPulseWidth = 5;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = TRUE;
			vmode->mInterlaced = TRUE;
			vmode->mPixelClock = 74250000;
			break;
		case 21:                   //720(1440)x576i @ 50Hz 4:3
			vmode->mHImageSize = 4;
			vmode->mVImageSize = 3;
		case 22:                   //720(1440)x576i @ 50Hz 16:9
			vmode->mHActive = 1440;
			vmode->mVActive = 288;
			vmode->mHBlanking = 288;
			vmode->mVBlanking = 24;
			vmode->mHSyncOffset = 24;
			vmode->mVSyncOffset = 2;
			vmode->mHSyncPulseWidth = 126;
			vmode->mVSyncPulseWidth = 3;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = FALSE;
			vmode->mInterlaced = TRUE;
			vmode->mPixelClock = 27000000;
			vmode->mPixelRepetitionInput = 1;
			break;
		case 23:                   //720(1440)x288p @ 50Hz 4:3
			vmode->mHImageSize = 4;
			vmode->mVImageSize = 3;
		case 24:                   //720(1440)x288p @ 50Hz 16:9
			vmode->mHActive = 1440;
			vmode->mVActive = 288;
			vmode->mHBlanking = 288;
			vmode->mVBlanking = 	(vmode->mRefreshRate == 50080) ? 24 : ((vmode->mRefreshRate == 49920) ? 25 : 26);
			vmode->mHSyncOffset = 24;
			vmode->mVSyncOffset = (vmode->mRefreshRate == 50080) ? 2 : ((vmode->mRefreshRate == 49920) ? 3 : 4);
			vmode->mHSyncPulseWidth = 126;
			vmode->mVSyncPulseWidth = 3;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = FALSE;
			vmode->mInterlaced = FALSE;
			vmode->mPixelClock = 27000000;
			vmode->mPixelRepetitionInput = 1;
			break;
		case 25:                   //2880x576i @ 50Hz 4:3
			vmode->mHImageSize = 4;
			vmode->mVImageSize = 3;
		case 26:                   //2880x576i @ 50Hz 16:9
			vmode->mHActive = 2880;
			vmode->mVActive = 288;
			vmode->mHBlanking = 576;
			vmode->mVBlanking = 24;
			vmode->mHSyncOffset = 48;
			vmode->mVSyncOffset = 2;
			vmode->mHSyncPulseWidth = 252;
			vmode->mVSyncPulseWidth = 3;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = FALSE;
			vmode->mInterlaced = TRUE;
			vmode->mPixelClock = 54000000;
			break;
		case 27:                   //2880x288p @ 50Hz 4:3
			vmode->mHImageSize = 4;
			vmode->mVImageSize = 3;
		case 28:                   //2880x288p @ 50Hz 16:9
			vmode->mHActive = 2880;
			vmode->mVActive = 288;
			vmode->mHBlanking = 576;
			vmode->mVBlanking = (vmode->mRefreshRate == 50080) ? 24 : ((vmode->mRefreshRate == 49920) ? 25 : 26);
			vmode->mHSyncOffset = 48;
			vmode->mVSyncOffset = (vmode->mRefreshRate == 50080) ? 2 : ((vmode->mRefreshRate == 49920) ? 3 : 4);
			vmode->mHSyncPulseWidth = 252;
			vmode->mVSyncPulseWidth = 3;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = FALSE;
			vmode->mInterlaced = FALSE;
			vmode->mPixelClock = 54000000;
			break;
		case 29:                   //1440x576p @ 50Hz 4:3
			vmode->mHImageSize = 4;
			vmode->mVImageSize = 3;
		case 30:                   //1440x576p @ 50Hz 16:9
			vmode->mHActive = 1440;
			vmode->mVActive = 576;
			vmode->mHBlanking = 288;
			vmode->mVBlanking = 49;
			vmode->mHSyncOffset = 24;
			vmode->mVSyncOffset = 5;
			vmode->mHSyncPulseWidth = 128;
			vmode->mVSyncPulseWidth = 5;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = FALSE;
			vmode->mInterlaced = FALSE;
			vmode->mPixelClock = 54000000;
			break;
		case 31:                   //1920x1080p @ 50Hz 16:9
			vmode->mHActive = 1920;
			vmode->mVActive = 1080;
			vmode->mHBlanking = 720;
			vmode->mVBlanking = 45;
			vmode->mHSyncOffset = 528;
			vmode->mVSyncOffset = 4;
			vmode->mHSyncPulseWidth = 44;
			vmode->mVSyncPulseWidth = 5;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = TRUE;
			vmode->mInterlaced = FALSE;
			vmode->mPixelClock = 148500000;
			break;
		case 32:                   //1920x1080p @ 23.976/24Hz 16:9
			vmode->mHActive = 1920;
			vmode->mVActive = 1080;
			vmode->mHBlanking = 830;
			vmode->mVBlanking = 45;
			vmode->mHSyncOffset = 638;
			vmode->mVSyncOffset = 4;
			vmode->mHSyncPulseWidth = 44;
			vmode->mVSyncPulseWidth = 5;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = TRUE;
			vmode->mInterlaced = FALSE;
			vmode->mPixelClock = (vmode->mRefreshRate == 23976) ? 74170000 : 74250000;
			break;
		case 33:                   //1920x1080p @ 25Hz 16:9
			vmode->mHActive = 1920;
			vmode->mVActive = 1080;
			vmode->mHBlanking = 720;
			vmode->mVBlanking = 45;
			vmode->mHSyncOffset = 528;
			vmode->mVSyncOffset = 4;
			vmode->mHSyncPulseWidth = 44;
			vmode->mVSyncPulseWidth = 5;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = TRUE;
			vmode->mInterlaced = FALSE;
			vmode->mPixelClock = 74250000;
			break;
		case 34:                   //1920x1080p @ 29.97/30Hz 16:9
			vmode->mHActive = 1920;
			vmode->mVActive = 1080;
			vmode->mHBlanking = 280;
			vmode->mVBlanking = 45;
			vmode->mHSyncOffset = 88;
			vmode->mVSyncOffset = 4;
			vmode->mHSyncPulseWidth = 44;
			vmode->mVSyncPulseWidth = 5;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = TRUE;
			vmode->mInterlaced = FALSE;
			vmode->mPixelClock = (vmode->mRefreshRate == 29970) ? 74170000 : 74250000;
			break;
		case 35:                   //2880x480p @ 60Hz 4:3
			vmode->mHImageSize = 4;
			vmode->mVImageSize = 3;
		case 36:                   //2880x480p @ 60Hz 16:9
			vmode->mHActive = 2880;
			vmode->mVActive = 480;
			vmode->mHBlanking = 552;
			vmode->mVBlanking = 45;
			vmode->mHSyncOffset = 64;
			vmode->mVSyncOffset = 9;
			vmode->mHSyncPulseWidth = 248;
			vmode->mVSyncPulseWidth = 6;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = FALSE;
			vmode->mInterlaced = FALSE;
			vmode->mPixelClock = (vmode->mRefreshRate == 59940) ? 108000000 : 108100000;
			break;
		case 37:                   //2880x576p @ 50Hz 4:3
			vmode->mHImageSize = 4;
			vmode->mVImageSize = 3;
		case 38:                   //2880x576p @ 50Hz 16:9
			vmode->mHActive = 2880;
			vmode->mVActive = 576;
			vmode->mHBlanking = 576;
			vmode->mVBlanking = 49;
			vmode->mHSyncOffset = 48;
			vmode->mVSyncOffset = 5;
			vmode->mHSyncPulseWidth = 256;
			vmode->mVSyncPulseWidth = 5;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = FALSE;
			vmode->mInterlaced = FALSE;
			vmode->mPixelClock = 108000000;
			break;
		case 39:                   //1920x1080i (1250 total) @ 50Hz 16:9
			vmode->mHActive = 1920;
			vmode->mVActive = 540;
			vmode->mHBlanking = 384;
			vmode->mVBlanking = 85;
			vmode->mHSyncOffset = 32;
			vmode->mVSyncOffset = 23;
			vmode->mHSyncPulseWidth = 168;
			vmode->mVSyncPulseWidth = 5;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = TRUE;
			vmode->mInterlaced = TRUE;
			vmode->mPixelClock = 72000000;
			break;
		case 40:                   //1920x1080i @ 100Hz 16:9
			vmode->mHActive = 1920;
			vmode->mVActive = 540;
			vmode->mHBlanking = 720;
			vmode->mVBlanking = 22;
			vmode->mHSyncOffset = 528;
			vmode->mVSyncOffset = 2;
			vmode->mHSyncPulseWidth = 44;
			vmode->mVSyncPulseWidth = 5;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = TRUE;
			vmode->mInterlaced = TRUE;
			vmode->mPixelClock = 148500000;
			break;
		case 41:                   //1280x720p @ 100Hz 16:9
			vmode->mHActive = 1280;
			vmode->mVActive = 720;
			vmode->mHBlanking = 700;
			vmode->mVBlanking = 30;
			vmode->mHSyncOffset = 440;
			vmode->mVSyncOffset = 5;
			vmode->mHSyncPulseWidth = 40;
			vmode->mVSyncPulseWidth = 5;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = TRUE;
			vmode->mInterlaced = FALSE;
			vmode->mPixelClock = 148500000;
			break;
		case 42:                   //720x576p @ 100Hz 4:3
			vmode->mHImageSize = 4;
			vmode->mVImageSize = 3;
		case 43:                   //720x576p @ 100Hz 16:9
			vmode->mHActive = 720;
			vmode->mVActive = 576;
			vmode->mHBlanking = 144;
			vmode->mVBlanking = 49;
			vmode->mHSyncOffset = 12;
			vmode->mVSyncOffset = 5;
			vmode->mHSyncPulseWidth = 64;
			vmode->mVSyncPulseWidth = 5;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = FALSE;
			vmode->mInterlaced = FALSE;
			vmode->mPixelClock = 54000000;
			break;
		case 44:                   //720(1440)x576i @ 100Hz 4:3
			vmode->mHImageSize = 4;
			vmode->mVImageSize = 3;
		case 45:                   //720(1440)x576i @ 100Hz 16:9
			vmode->mHActive = 1440;
			vmode->mVActive = 288;
			vmode->mHBlanking = 288;
			vmode->mVBlanking = 24;
			vmode->mHSyncOffset = 24;
			vmode->mVSyncOffset = 2;
			vmode->mHSyncPulseWidth = 126;
			vmode->mVSyncPulseWidth = 3;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = FALSE;
			vmode->mInterlaced = TRUE;
			vmode->mPixelClock = 54000000;
			vmode->mPixelRepetitionInput = 1;
			break;
		case 46:                   //1920x1080i @ 119.88/120Hz 16:9
			vmode->mHActive = 1920;
			vmode->mVActive = 540;
			vmode->mHBlanking = 288;
			vmode->mVBlanking = 22;
			vmode->mHSyncOffset = 88;
			vmode->mVSyncOffset = 2;
			vmode->mHSyncPulseWidth = 44;
			vmode->mVSyncPulseWidth = 5;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = TRUE;
			vmode->mInterlaced = TRUE;
			vmode->mPixelClock = (vmode->mRefreshRate == 119880) ? 148350000 : 148500000;
			break;
		case 47:                   //1280x720p @ 119.88/120Hz 16:9
			vmode->mHActive = 1280;
			vmode->mVActive = 720;
			vmode->mHBlanking = 370;
			vmode->mVBlanking = 30;
			vmode->mHSyncOffset = 110;
			vmode->mVSyncOffset = 5;
			vmode->mHSyncPulseWidth = 40;
			vmode->mVSyncPulseWidth = 5;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = TRUE;
			vmode->mInterlaced = FALSE;
			vmode->mPixelClock = (vmode->mRefreshRate == 119880) ? 148350000 : 148500000;
			break;
		case 48:                   //720x480p @ 119.88/120Hz 4:3
			vmode->mHImageSize = 4;
			vmode->mVImageSize = 3;
		case 49:                   //720x480p @ 119.88/120Hz 16:9
			vmode->mHActive = 720;
			vmode->mVActive = 480;
			vmode->mHBlanking = 138;
			vmode->mVBlanking = 45;
			vmode->mHSyncOffset = 16;
			vmode->mVSyncOffset = 9;
			vmode->mHSyncPulseWidth = 62;
			vmode->mVSyncPulseWidth = 6;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = FALSE;
			vmode->mInterlaced = FALSE;
			vmode->mPixelClock = (vmode->mRefreshRate == 119880) ? 54000000 : 54050000;
			break;
		case 50:                   //720(1440)x480i @ 119.88/120Hz 4:3
			vmode->mHImageSize = 4;
			vmode->mVImageSize = 3;
		case 51:                   //720(1440)x480i @ 119.88/120Hz 16:9
			vmode->mHActive = 1440;
			vmode->mVActive = 240;
			vmode->mHBlanking = 276;
			vmode->mVBlanking = 22;
			vmode->mHSyncOffset = 38;
			vmode->mVSyncOffset = 4;
			vmode->mHSyncPulseWidth = 124;
			vmode->mVSyncPulseWidth = 3;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = FALSE;
			vmode->mInterlaced = TRUE;
			vmode->mPixelClock = (vmode->mRefreshRate == 119880) ? 54000000 : 54050000;
			vmode->mPixelRepetitionInput = 1;
			break;
		case 52:                   //720X576p @ 200Hz 4:3
			vmode->mHImageSize = 4;
			vmode->mVImageSize = 3;
		case 53:                   //720X576p @ 200Hz 16:9
			vmode->mHActive = 720;
			vmode->mVActive = 576;
			vmode->mHBlanking = 144;
			vmode->mVBlanking = 49;
			vmode->mHSyncOffset = 12;
			vmode->mVSyncOffset = 5;
			vmode->mHSyncPulseWidth = 64;
			vmode->mVSyncPulseWidth = 5;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = FALSE;
			vmode->mInterlaced = FALSE;
			vmode->mPixelClock = 108000000;
			break;
		case 54:                   //720(1440)x576i @ 200Hz 4:3
			vmode->mHImageSize = 4;
			vmode->mVImageSize = 3;
		case 55:                   //720(1440)x576i @ 200Hz 16:9
			vmode->mHActive = 1440;
			vmode->mVActive = 288;
			vmode->mHBlanking = 288;
			vmode->mVBlanking = 24;
			vmode->mHSyncOffset = 24;
			vmode->mVSyncOffset = 2;
			vmode->mHSyncPulseWidth = 126;
			vmode->mVSyncPulseWidth = 3;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = FALSE;
			vmode->mInterlaced = TRUE;
			vmode->mPixelClock = 108000000;
			vmode->mPixelRepetitionInput = 1;
			break;
		case 56:                   //720x480p @ 239.76/240Hz 4:3
			vmode->mHImageSize = 4;
			vmode->mVImageSize = 3;
		case 57:                   //720x480p @ 239.76/240Hz 16:9
			vmode->mHActive = 720;
			vmode->mVActive = 480;
			vmode->mHBlanking = 138;
			vmode->mVBlanking = 45;
			vmode->mHSyncOffset = 16;
			vmode->mVSyncOffset = 9;
			vmode->mHSyncPulseWidth = 62;
			vmode->mVSyncPulseWidth = 6;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = FALSE;
			vmode->mInterlaced = FALSE;
			vmode->mPixelClock = (vmode->mRefreshRate == 239760) ? 108000000 : 108100000;
			break;
		case 58:                   //720(1440)x480i @ 239.76/240Hz 4:3
			vmode->mHImageSize = 4;
			vmode->mVImageSize = 3;
		case 59:                   //720(1440)x480i @ 239.76/240Hz 16:9
			vmode->mHActive = 1440;
			vmode->mVActive = 240;
			vmode->mHBlanking = 276;
			vmode->mVBlanking = 22;
			vmode->mHSyncOffset = 38;
			vmode->mVSyncOffset = 4;
			vmode->mHSyncPulseWidth = 124;
			vmode->mVSyncPulseWidth = 3;
			vmode->mHSyncPolarity = vmode->mVSyncPolarity = FALSE;
			vmode->mInterlaced = TRUE;
			vmode->mPixelClock = (vmode->mRefreshRate == 239760) ? 108000000 : 108100000;
			vmode->mPixelRepetitionInput = 1;
			break;
		default:
			vmode->mCode = -1;
			printf("invalid code\n");
			break;
	}
}

/*!
 * HDMI display enable.
 */
void hdmi_display_enable(unsigned int mCode)
{
	hdmi_instance.enc_in_format = eRGB;
	hdmi_instance.enc_out_format = eRGB;
	hdmi_instance.enc_color_depth = 8;
	hdmi_instance.pix_repet_factor = 0;
	hdmi_instance.hdcp_enable = 0;
	hdmi_instance.video_mode.mCode = mCode;
	hdmi_instance.video_mode.mDVI = FALSE;
	hdmi_instance.video_mode.mRefreshRate = 60000;
	hdmi_instance.video_mode.mDataEnablePolarity = TRUE;

	hdmi_set_video_mode(&hdmi_instance);
	hdmi_av_composer(&hdmi_instance);
	hdmi_phy_init(&hdmi_instance);
	hdmi_enable_video_path(&hdmi_instance);
	hdmi_enable_audio_clk(&hdmi_instance, FALSE);
	hdmi_audio_mute(&hdmi_instance, TRUE);
	hdmi_config_AVI(&hdmi_instance);
	hdmi_video_packetize(&hdmi_instance);
	hdmi_video_csc(&hdmi_instance);
	hdmi_video_sample(&hdmi_instance);
	hdmi_tx_hdcp_config(&hdmi_instance);
}

