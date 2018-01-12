/*
 * Copyright (C) 2012-2016 Freescale Semiconductor, Inc.
 *
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/errno.h>
#include <asm/gpio.h>
#include <asm/imx-common/mxc_i2c.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/imx-common/video.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/arch/crm_regs.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <i2c.h>
#include <power/pmic.h>
#include <power/pfuze100_pmic.h>
#include "../common/pfuze.h"
#include <asm/arch/mx6-ddr.h>
#include <usb.h>
#if defined(CONFIG_MX6DL) && defined(CONFIG_MXC_EPDC)
#include <lcd.h>
#include <mxc_epdc_fb.h>
#endif
#ifdef CONFIG_CMD_SATA
#include <asm/imx-common/sata.h>
#endif
#ifdef CONFIG_FSL_FASTBOOT
#include <fsl_fastboot.h>
#ifdef CONFIG_ANDROID_RECOVERY
#include <recovery.h>
#endif
#endif /*CONFIG_FSL_FASTBOOT*/

#ifdef CONFIG_UBOOT_LOGO_ENABLE_OVER_8BITS
#include <asm/imx-common/imx_pwm.h>
#include <asm/imx-common/mxc_ipu.h>
#endif

#include "eeprom_info.h"

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define SPI_PAD_CTRL (PAD_CTL_HYS | PAD_CTL_SPEED_MED | \
		      PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)

#define I2C_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS |	\
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)

#define EPDC_PAD_CTRL    (PAD_CTL_PKE | PAD_CTL_SPEED_MED |	\
	PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define OTG_ID_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW |		\
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)


#define I2C_PMIC	1

#define I2C_PAD MUX_PAD_CTRL(I2C_PAD_CTRL)

#define DISP0_PWR_EN	IMX_GPIO_NR(1, 21)
#define LVDS_VCC_PORT1	IMX_GPIO_NR(3, 16)
#define LVDS_VCC_PORT2	IMX_GPIO_NR(3, 17)

extern unsigned char fsl_bmp_reversed_600x400[];
extern int fsl_bmp_reversed_600x400_size;
extern int g_ipu_hw_rev;

extern int video_display_bitmap(ulong bmp_image, int x, int y);
#ifndef GPIO_GDIR
#define GPIO_GDIR	4
#endif
#ifndef GPIO_DR
#define GPIO_DR		0
#endif
void set_LVDS_VCC(int status)
{
	unsigned int reg;
	//puts("set_LVDS_VCC\n");
	if(status)
	{
		reg=readl(GPIO3_BASE_ADDR+ GPIO_GDIR);
		reg |= (3<<16);
		writel(reg, GPIO3_BASE_ADDR+ GPIO_GDIR);
		
		reg=readl(GPIO3_BASE_ADDR+ GPIO_DR);
		reg |= (3<<16);
		writel(reg, GPIO3_BASE_ADDR+ GPIO_DR);
	}
	else
	{
		imx_iomux_v3_setup_pad(MX6_PAD_EIM_D16__GPIO3_IO16 | MUX_PAD_CTRL(NO_PAD_CTRL));
		imx_iomux_v3_setup_pad(MX6_PAD_EIM_D17__GPIO3_IO17 | MUX_PAD_CTRL(NO_PAD_CTRL));
		reg=readl(GPIO3_BASE_ADDR+ GPIO_DR);
		reg &= ~(3<<16);
		writel(reg, GPIO3_BASE_ADDR+ GPIO_DR);
		
		reg=readl(GPIO3_BASE_ADDR+ GPIO_GDIR);
		reg |= (3<<16);
		writel(reg, GPIO3_BASE_ADDR+ GPIO_GDIR);

		reg=readl(GPIO3_BASE_ADDR+ GPIO_DR);
		reg &= ~(3<<16);
		writel(reg, GPIO3_BASE_ADDR+ GPIO_DR);
	}
	return ;
}

void bmp_data_tranfer(char * destaddr, u32 value)
{
	u32 ovalue=0x0+value;

	while(ovalue>0)
	{
		*destaddr=(uchar)ovalue&0xff;
		ovalue=ovalue>>8;
		destaddr++;
	}
	*destaddr=0x0;
}

void set_bmp_header(char * destaddr, int width, int high)
{
	bmp_data_tranfer(destaddr+2, 0x436+width*high);
	bmp_data_tranfer(destaddr+18, width);
	bmp_data_tranfer(destaddr+22, high);
	return;
}

int get_bmp_4byte(char * srcaddr)
{
	int ret=0;
	int i;
	for(i=0;i<4;i++)
	{
		ret=ret*256+srcaddr[3-i];
	}
	return ret;
}

void set_panel_env(void)
{
	char tmpchar[128]={0};
	set_LVDS_VCC(1);
	uchar color_depth=eeprom_i2c_get_color_depth();
	switch(eeprom_i2c_get_EDID())
	{
		case RESOLUTION_640X480:
			sprintf(tmpchar,"panel640x480d%d",color_depth);
			break;
		case RESOLUTION_800X480:
			sprintf(tmpchar,"panel800x480d%d",color_depth);
			break;
		case RESOLUTION_800X600:
			sprintf(tmpchar,"panel800x600d%d",color_depth);
			break;
		case RESOLUTION_1024X600:
			sprintf(tmpchar,"panel1024x600d%d",color_depth);
			break;
		case RESOLUTION_1024X768:
			sprintf(tmpchar,"panel1024x768d%d",color_depth);
			break;
		case RESOLUTION_1280X800:
			sprintf(tmpchar,"panel1280x800d%d",color_depth);
			break;
		case RESOLUTION_1366X768:
			sprintf(tmpchar,"panel1366x768d%d",color_depth);
			break;
		case RESOLUTION_1920X1080:
			sprintf(tmpchar,"panel1920x1080d%d",color_depth);
			break;
		default:
			sprintf(tmpchar,"panel800x480d18");
			puts("set_panel_env error\n"); 
			break;
	}
	setenv("panel",tmpchar);
	setenv("lvds_num","1");//lvds1 route to di 1
	setenv("disp_num","1");//lvds1 route to di 1
}

void copy_bmp_screen(char * destaddr,char * srcaddr, int width,int high)
{
	u32 offset=0x0;
	int i=0;
	int bmpwidth=280;
	int bmphigh=168;
	extern unsigned char fsl_bmp_reversed_pass[];
//	char tmp[128]={0};

	memset((char *)destaddr, 0x0, (width+2)*high+0x436);
	if(eeprom_i2c_pass_logo()==0)
	{
		bmpwidth=get_bmp_4byte((char *) srcaddr+18);
		bmphigh=get_bmp_4byte((char *) srcaddr+22);
		memcpy((char *)destaddr, (char *)srcaddr,0x436);
	}
	else
	{
		bmpwidth=get_bmp_4byte((char *) fsl_bmp_reversed_pass+18);
		bmphigh=get_bmp_4byte((char *) fsl_bmp_reversed_pass+22);
		memcpy((char *)destaddr, (char *)fsl_bmp_reversed_pass,0x436);
	}
//	sprintf(tmp,"(%d,%d)\n",bmpwidth,bmphigh);
//	puts(tmp);
	set_bmp_header(destaddr,width,high);
	if(width==1366)width=1368;
	for(i=0;i<bmphigh;i++)
	{
		if(width==1920)
			offset=0x436+width*(high/2-bmphigh/2+i)+(width -bmpwidth)/2-430;
		else
			offset=0x436+width*(high/2-bmphigh/2+i)+(width -bmpwidth)/2;
		if(eeprom_i2c_pass_logo()==0)
			memcpy((char *)destaddr+offset, (char *)srcaddr+(u32)(0x436+bmpwidth*i),bmpwidth);
		else
			memcpy((char *)destaddr+offset, (char *)fsl_bmp_reversed_pass+(u32)(0x436+bmpwidth*i),bmpwidth);
	}
}

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();
	return 0;
}

static iomux_v3_cfg_t const uart1_pads[] = {
	MX6_PAD_CSI0_DAT10__UART1_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI0_DAT11__UART1_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const enet_pads[] = {
	MX6_PAD_ENET_MDIO__ENET_MDIO		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_MDC__ENET_MDC		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TXC__RGMII_TXC	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD0__RGMII_TD0	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD1__RGMII_TD1	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD2__RGMII_TD2	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TD3__RGMII_TD3	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_TX_CTL__RGMII_TX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET_REF_CLK__ENET_TX_CLK	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RXC__RGMII_RXC	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD0__RGMII_RD0	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD1__RGMII_RD1	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD2__RGMII_RD2	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RD3__RGMII_RD3	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_RGMII_RX_CTL__RGMII_RX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	/* AR8031 PHY Reset */
	MX6_PAD_ENET_CRS_DV__GPIO1_IO25		| MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void setup_iomux_enet(void)
{
	imx_iomux_v3_setup_multiple_pads(enet_pads, ARRAY_SIZE(enet_pads));

	/* Reset AR8031 PHY */
	gpio_direction_output(IMX_GPIO_NR(1, 25) , 0);
	mdelay(10);
	gpio_set_value(IMX_GPIO_NR(1, 25), 1);
	udelay(100);
}

static iomux_v3_cfg_t const usdhc2_pads[] = {
	MX6_PAD_SD2_CLK__SD2_CLK	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_CMD__SD2_CMD	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT0__SD2_DATA0	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT1__SD2_DATA1	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT2__SD2_DATA2	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD2_DAT3__SD2_DATA3	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	//MX6_PAD_NANDF_D4__SD2_DATA4	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	//MX6_PAD_NANDF_D5__SD2_DATA5	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	//MX6_PAD_NANDF_D6__SD2_DATA6	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	//MX6_PAD_NANDF_D7__SD2_DATA7	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	//MX6_PAD_NANDF_D2__GPIO2_IO02	| MUX_PAD_CTRL(NO_PAD_CTRL), /* CD */
};

static iomux_v3_cfg_t const usdhc3_pads[] = {
	MX6_PAD_SD3_CLK__SD3_CLK   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_CMD__SD3_CMD   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT0__SD3_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT1__SD3_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT2__SD3_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT3__SD3_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	//MX6_PAD_SD3_DAT4__SD3_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	//MX6_PAD_SD3_DAT5__SD3_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	//MX6_PAD_SD3_DAT6__SD3_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	//MX6_PAD_SD3_DAT7__SD3_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	//MX6_PAD_NANDF_D0__GPIO2_IO00    | MUX_PAD_CTRL(NO_PAD_CTRL), /* CD */
};

static iomux_v3_cfg_t const usdhc4_pads[] = {
	MX6_PAD_SD4_CLK__SD4_CLK   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_CMD__SD4_CMD   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT0__SD4_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT1__SD4_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT2__SD4_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT3__SD4_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT4__SD4_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT5__SD4_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT6__SD4_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD4_DAT7__SD4_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
};

#if 0
//#ifdef CONFIG_MXC_SPI
static iomux_v3_cfg_t const ecspi1_pads[] = {
	MX6_PAD_KEY_COL0__ECSPI1_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_KEY_COL1__ECSPI1_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_KEY_ROW0__ECSPI1_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_KEY_ROW1__GPIO4_IO09 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void setup_spi(void)
{
	imx_iomux_v3_setup_multiple_pads(ecspi1_pads, ARRAY_SIZE(ecspi1_pads));
}

int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	return (bus == 0 && cs == 0) ? (IMX_GPIO_NR(4, 9)) : -1;
}
#endif

#if 0
static iomux_v3_cfg_t const rgb_pads[] = {
	MX6_PAD_DI0_DISP_CLK__IPU1_DI0_DISP_CLK | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DI0_PIN15__IPU1_DI0_PIN15 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DI0_PIN2__IPU1_DI0_PIN02 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DI0_PIN3__IPU1_DI0_PIN03 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DI0_PIN4__IPU1_DI0_PIN04 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT0__IPU1_DISP0_DATA00 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT1__IPU1_DISP0_DATA01 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT2__IPU1_DISP0_DATA02 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT3__IPU1_DISP0_DATA03 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT4__IPU1_DISP0_DATA04 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT5__IPU1_DISP0_DATA05 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT6__IPU1_DISP0_DATA06 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT7__IPU1_DISP0_DATA07 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT8__IPU1_DISP0_DATA08 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT9__IPU1_DISP0_DATA09 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT10__IPU1_DISP0_DATA10 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT11__IPU1_DISP0_DATA11 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT12__IPU1_DISP0_DATA12 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT13__IPU1_DISP0_DATA13 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT14__IPU1_DISP0_DATA14 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT15__IPU1_DISP0_DATA15 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT16__IPU1_DISP0_DATA16 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT17__IPU1_DISP0_DATA17 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT18__IPU1_DISP0_DATA18 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT19__IPU1_DISP0_DATA19 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT20__IPU1_DISP0_DATA20 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT21__IPU1_DISP0_DATA21 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT22__IPU1_DISP0_DATA22 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_DISP0_DAT23__IPU1_DISP0_DATA23 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_SD1_DAT3__GPIO1_IO21 | MUX_PAD_CTRL(NO_PAD_CTRL),
};
static void enable_rgb(struct display_info_t const *dev)
{
	imx_iomux_v3_setup_multiple_pads(rgb_pads, ARRAY_SIZE(rgb_pads));
	gpio_direction_output(DISP0_PWR_EN, 1);
}
#endif
static struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = MX6_PAD_KEY_COL3__I2C2_SCL | I2C_PAD,
		.gpio_mode = MX6_PAD_KEY_COL3__GPIO4_IO12 | I2C_PAD,
		.gp = IMX_GPIO_NR(4, 12)
	},
	.sda = {
		.i2c_mode = MX6_PAD_KEY_ROW3__I2C2_SDA | I2C_PAD,
		.gpio_mode = MX6_PAD_KEY_ROW3__GPIO4_IO13 | I2C_PAD,
		.gp = IMX_GPIO_NR(4, 13)
	}
};

#ifndef CONFIG_EEPROM_GPIO_I2C4
static struct i2c_pads_info i2c_pad_info2 = {
	.scl = {
		.i2c_mode = MX6_PAD_GPIO_3__I2C3_SCL | I2C_PAD,
		.gpio_mode = MX6_PAD_GPIO_3__GPIO1_IO03 | I2C_PAD,
		.gp = IMX_GPIO_NR(1, 3)
	},
	.sda = {
		.i2c_mode = MX6_PAD_GPIO_6__I2C3_SDA | I2C_PAD,
		.gpio_mode =  MX6_PAD_GPIO_6__GPIO1_IO06 | I2C_PAD,
		.gp = IMX_GPIO_NR(1, 6)
	}
};
#endif
iomux_v3_cfg_t const pcie_pads[] = {
	MX6_PAD_EIM_D19__GPIO3_IO19 | MUX_PAD_CTRL(NO_PAD_CTRL),	/* POWER */
	MX6_PAD_GPIO_17__GPIO7_IO12 | MUX_PAD_CTRL(NO_PAD_CTRL),	/* RESET */
};

static void setup_pcie(void)
{
	imx_iomux_v3_setup_multiple_pads(pcie_pads, ARRAY_SIZE(pcie_pads));
}

iomux_v3_cfg_t const di0_pads[] = {
	MX6_PAD_DI0_DISP_CLK__IPU1_DI0_DISP_CLK,	/* DISP0_CLK */
	MX6_PAD_DI0_PIN2__IPU1_DI0_PIN02,		/* DISP0_HSYNC */
	MX6_PAD_DI0_PIN3__IPU1_DI0_PIN03,		/* DISP0_VSYNC */
};

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
}

#if defined(CONFIG_MX6DL) && defined(CONFIG_MXC_EPDC)
static iomux_v3_cfg_t const epdc_enable_pads[] = {
	MX6_PAD_EIM_A16__EPDC_DATA00	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EIM_DA10__EPDC_DATA01	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EIM_DA12__EPDC_DATA02	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EIM_DA11__EPDC_DATA03	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EIM_LBA__EPDC_DATA04	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EIM_EB2__EPDC_DATA05	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EIM_CS0__EPDC_DATA06	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EIM_RW__EPDC_DATA07	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EIM_A21__EPDC_GDCLK	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EIM_A22__EPDC_GDSP	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EIM_A23__EPDC_GDOE	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EIM_A24__EPDC_GDRL	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EIM_D31__EPDC_SDCLK_P	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EIM_D27__EPDC_SDOE	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EIM_DA1__EPDC_SDLE	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EIM_EB1__EPDC_SDSHR	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EIM_DA2__EPDC_BDR0	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EIM_DA4__EPDC_SDCE0	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EIM_DA5__EPDC_SDCE1	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX6_PAD_EIM_DA6__EPDC_SDCE2	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
};

static iomux_v3_cfg_t const epdc_disable_pads[] = {
	MX6_PAD_EIM_A16__GPIO2_IO22,
	MX6_PAD_EIM_DA10__GPIO3_IO10,
	MX6_PAD_EIM_DA12__GPIO3_IO12,
	MX6_PAD_EIM_DA11__GPIO3_IO11,
	MX6_PAD_EIM_LBA__GPIO2_IO27,
	MX6_PAD_EIM_EB2__GPIO2_IO30,
	MX6_PAD_EIM_CS0__GPIO2_IO23,
	MX6_PAD_EIM_RW__GPIO2_IO26,
	MX6_PAD_EIM_A21__GPIO2_IO17,
	MX6_PAD_EIM_A22__GPIO2_IO16,
	MX6_PAD_EIM_A23__GPIO6_IO06,
	MX6_PAD_EIM_A24__GPIO5_IO04,
	MX6_PAD_EIM_D31__GPIO3_IO31,
	MX6_PAD_EIM_D27__GPIO3_IO27,
	MX6_PAD_EIM_DA1__GPIO3_IO01,
	MX6_PAD_EIM_EB1__GPIO2_IO29,
	MX6_PAD_EIM_DA2__GPIO3_IO02,
	MX6_PAD_EIM_DA4__GPIO3_IO04,
	MX6_PAD_EIM_DA5__GPIO3_IO05,
	MX6_PAD_EIM_DA6__GPIO3_IO06,
};
#endif

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg usdhc_cfg[3] = {
	{USDHC2_BASE_ADDR},
	{USDHC3_BASE_ADDR},
	{USDHC4_BASE_ADDR},
};


int board_mmc_get_env_dev(int devno)
{
	return devno - 1;
}

int mmc_map_to_kernel_blk(int devno)
{
	return devno + 1;
}

#ifndef CONFIG_SBC7112
#define USDHC2_CD_GPIO	IMX_GPIO_NR(2, 2)
#define USDHC3_CD_GPIO	IMX_GPIO_NR(2, 0)
#else
//#define USDHC2_CD_GPIO	IMX_GPIO_NR(2, 2)
#define USDHC3_CD_GPIO	IMX_GPIO_NR(4, 10)//mmc cd by aplex 
#endif

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC2_BASE_ADDR:
#ifndef CONFIG_SBC7112
		ret = !gpio_get_value(USDHC2_CD_GPIO);
#else
		ret =0; /* aplex */
#endif
		break;
	case USDHC3_BASE_ADDR:
		ret = !gpio_get_value(USDHC3_CD_GPIO);
		break;
	case USDHC4_BASE_ADDR:
		ret = 1; /* eMMC/uSDHC4 is always present */
		break;
	}

	return ret;
}

int board_mmc_init(bd_t *bis)
{
#ifndef CONFIG_SPL_BUILD
	int ret;
	int i;

	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-Boot device node)    (Physical Port)
	 * mmc0                    SD2
	 * mmc1                    SD3
	 * mmc2                    eMMC
	 */
	for (i = 0; i < CONFIG_SYS_FSL_USDHC_NUM; i++) {
		switch (i) {
		case 0:
			imx_iomux_v3_setup_multiple_pads(
				usdhc2_pads, ARRAY_SIZE(usdhc2_pads));
#ifndef CONFIG_SBC7112
			gpio_direction_input(USDHC2_CD_GPIO);
#endif
			usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
			break;
		case 1:
			imx_iomux_v3_setup_multiple_pads(
				usdhc3_pads, ARRAY_SIZE(usdhc3_pads));
			gpio_direction_input(USDHC3_CD_GPIO);
			usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
			break;
		case 2:
			imx_iomux_v3_setup_multiple_pads(
				usdhc4_pads, ARRAY_SIZE(usdhc4_pads));
			usdhc_cfg[2].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);
			break;
		default:
			printf("Warning: you configured more USDHC controllers"
			       "(%d) then supported by the board (%d)\n",
			       i + 1, CONFIG_SYS_FSL_USDHC_NUM);
			return -EINVAL;
		}

		ret = fsl_esdhc_initialize(bis, &usdhc_cfg[i]);
		if (ret)
			return ret;
	}

	Load_config_from_mmc();
	return 0;
#else
	struct src *psrc = (struct src *)SRC_BASE_ADDR;
	unsigned reg = readl(&psrc->sbmr1) >> 11;
	/*
	 * Upon reading BOOT_CFG register the following map is done:
	 * Bit 11 and 12 of BOOT_CFG register can determine the current
	 * mmc port
	 * 0x1                  SD1
	 * 0x2                  SD2
	 * 0x3                  SD4
	 */

	switch (reg & 0x3) {
	case 0x1:
		imx_iomux_v3_setup_multiple_pads(
			usdhc2_pads, ARRAY_SIZE(usdhc2_pads));
		usdhc_cfg[0].esdhc_base = USDHC2_BASE_ADDR;
		usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
		gd->arch.sdhc_clk = usdhc_cfg[0].sdhc_clk;
		break;
	case 0x2:
		imx_iomux_v3_setup_multiple_pads(
			usdhc3_pads, ARRAY_SIZE(usdhc3_pads));
		usdhc_cfg[0].esdhc_base = USDHC3_BASE_ADDR;
		usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
		gd->arch.sdhc_clk = usdhc_cfg[0].sdhc_clk;
		break;
	case 0x3:
		imx_iomux_v3_setup_multiple_pads(
			usdhc4_pads, ARRAY_SIZE(usdhc4_pads));
		usdhc_cfg[0].esdhc_base = USDHC4_BASE_ADDR;
		usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC4_CLK);
		gd->arch.sdhc_clk = usdhc_cfg[0].sdhc_clk;
		break;
	}

	return fsl_esdhc_initialize(bis, &usdhc_cfg[0]);
#endif
}
#endif

#if defined(CONFIG_MX6DL) && defined(CONFIG_MXC_EPDC)
vidinfo_t panel_info = {
	.vl_refresh = 85,
	.vl_col = 800,
	.vl_row = 600,
	.vl_pixclock = 26666667,
	.vl_left_margin = 8,
	.vl_right_margin = 100,
	.vl_upper_margin = 4,
	.vl_lower_margin = 8,
	.vl_hsync = 4,
	.vl_vsync = 1,
	.vl_sync = 0,
	.vl_mode = 0,
	.vl_flag = 0,
	.vl_bpix = 3,
	.cmap = 0,
};

struct epdc_timing_params panel_timings = {
	.vscan_holdoff = 4,
	.sdoed_width = 10,
	.sdoed_delay = 20,
	.sdoez_width = 10,
	.sdoez_delay = 20,
	.gdclk_hp_offs = 419,
	.gdsp_offs = 20,
	.gdoe_offs = 0,
	.gdclk_offs = 5,
	.num_ce = 1,
};

static void setup_epdc_power(void)
{
	/* Setup epdc voltage */

	/* EIM_A17 - GPIO2[21] for PWR_GOOD status */
	imx_iomux_v3_setup_pad(MX6_PAD_EIM_A17__GPIO2_IO21 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));
	/* Set as input */
	gpio_direction_input(IMX_GPIO_NR(2, 21));

	/* EIM_D17 - GPIO3[17] for VCOM control */
	imx_iomux_v3_setup_pad(MX6_PAD_EIM_D17__GPIO3_IO17 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));

	/* Set as output */
	gpio_direction_output(IMX_GPIO_NR(3, 17), 1);

	/* EIM_D20 - GPIO3[20] for EPD PMIC WAKEUP */
	imx_iomux_v3_setup_pad(MX6_PAD_EIM_D20__GPIO3_IO20 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));
	/* Set as output */
	gpio_direction_output(IMX_GPIO_NR(3, 20), 1);

	/* EIM_A18 - GPIO2[20] for EPD PWR CTL0 */
	imx_iomux_v3_setup_pad(MX6_PAD_EIM_A18__GPIO2_IO20 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));
	/* Set as output */
	gpio_direction_output(IMX_GPIO_NR(2, 20), 1);
}

static void epdc_enable_pins(void)
{
	/* epdc iomux settings */
	imx_iomux_v3_setup_multiple_pads(epdc_enable_pads,
				ARRAY_SIZE(epdc_enable_pads));
}

static void epdc_disable_pins(void)
{
	/* Configure MUX settings for EPDC pins to GPIO */
	imx_iomux_v3_setup_multiple_pads(epdc_disable_pads,
				ARRAY_SIZE(epdc_disable_pads));
}

static void setup_epdc(void)
{
	unsigned int reg;
	struct mxc_ccm_reg *ccm_regs = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/*** epdc Maxim PMIC settings ***/

	/* EPDC PWRSTAT - GPIO2[21] for PWR_GOOD status */
	imx_iomux_v3_setup_pad(MX6_PAD_EIM_A17__GPIO2_IO21 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));

	/* EPDC VCOM0 - GPIO3[17] for VCOM control */
	imx_iomux_v3_setup_pad(MX6_PAD_EIM_D17__GPIO3_IO17 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));

	/* UART4 TXD - GPIO3[20] for EPD PMIC WAKEUP */
	imx_iomux_v3_setup_pad(MX6_PAD_EIM_D20__GPIO3_IO20 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));

	/* EIM_A18 - GPIO2[20] for EPD PWR CTL0 */
	imx_iomux_v3_setup_pad(MX6_PAD_EIM_A18__GPIO2_IO20 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));

	/*** Set pixel clock rates for EPDC ***/

	/* EPDC AXI clk (IPU2_CLK) from PFD_400M, set to 396/2 = 198MHz */
	reg = readl(&ccm_regs->cscdr3);
	reg &= ~0x7C000;
	reg |= (1 << 16) | (1 << 14);
	writel(reg, &ccm_regs->cscdr3);

	/* EPDC AXI clk enable */
	reg = readl(&ccm_regs->CCGR3);
	reg |= 0x00C0;
	writel(reg, &ccm_regs->CCGR3);

	/* EPDC PIX clk (IPU2_DI1_CLK) from PLL5, set to 650/4/6 = ~27MHz */
	reg = readl(&ccm_regs->cscdr2);
	reg &= ~0x3FE00;
	reg |= (2 << 15) | (5 << 12);
	writel(reg, &ccm_regs->cscdr2);

	/* PLL5 enable (defaults to 650) */
	reg = readl(&ccm_regs->analog_pll_video);
	reg &= ~((1 << 16) | (1 << 12));
	reg |= (1 << 13);
	writel(reg, &ccm_regs->analog_pll_video);

	/* EPDC PIX clk enable */
	reg = readl(&ccm_regs->CCGR3);
	reg |= 0x0C00;
	writel(reg, &ccm_regs->CCGR3);

	panel_info.epdc_data.wv_modes.mode_init = 0;
	panel_info.epdc_data.wv_modes.mode_du = 1;
	panel_info.epdc_data.wv_modes.mode_gc4 = 3;
	panel_info.epdc_data.wv_modes.mode_gc8 = 2;
	panel_info.epdc_data.wv_modes.mode_gc16 = 2;
	panel_info.epdc_data.wv_modes.mode_gc32 = 2;

	panel_info.epdc_data.epdc_timings = panel_timings;

	setup_epdc_power();
}

void epdc_power_on(void)
{
	unsigned int reg;
	struct gpio_regs *gpio_regs = (struct gpio_regs *)GPIO2_BASE_ADDR;

	/* Set EPD_PWR_CTL0 to high - enable EINK_VDD (3.15) */
	gpio_set_value(IMX_GPIO_NR(2, 20), 1);
	udelay(1000);

	/* Enable epdc signal pin */
	epdc_enable_pins();

	/* Set PMIC Wakeup to high - enable Display power */
	gpio_set_value(IMX_GPIO_NR(3, 20), 1);

	/* Wait for PWRGOOD == 1 */
	while (1) {
		reg = readl(&gpio_regs->gpio_psr);
		if (!(reg & (1 << 21)))
			break;

		udelay(100);
	}

	/* Enable VCOM */
	gpio_set_value(IMX_GPIO_NR(3, 17), 1);

	udelay(500);
}

void epdc_power_off(void)
{
	/* Set PMIC Wakeup to low - disable Display power */
	gpio_set_value(IMX_GPIO_NR(3, 20), 0);

	/* Disable VCOM */
	gpio_set_value(IMX_GPIO_NR(3, 17), 0);

	epdc_disable_pins();

	/* Set EPD_PWR_CTL0 to low - disable EINK_VDD (3.15) */
	gpio_set_value(IMX_GPIO_NR(2, 20), 0);
}
#endif

int mx6_rgmii_rework(struct phy_device *phydev)
{
	unsigned short val;

	/* To enable AR8031 ouput a 125MHz clk from CLK_25M */
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x7);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, 0x8016);
	phy_write(phydev, MDIO_DEVAD_NONE, 0xd, 0x4007);

	val = phy_read(phydev, MDIO_DEVAD_NONE, 0xe);
	val &= 0xffe3;
	val |= 0x18;
	phy_write(phydev, MDIO_DEVAD_NONE, 0xe, val);

	/* introduce tx clock delay */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x5);
	val = phy_read(phydev, MDIO_DEVAD_NONE, 0x1e);
	val |= 0x0100;
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, val);

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	mx6_rgmii_rework(phydev);

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

#if defined(CONFIG_VIDEO_IPUV3)
static void disable_lvds(struct display_info_t const *dev)
{
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;

	int reg = readl(&iomux->gpr[2]);

	reg &= ~(IOMUXC_GPR2_LVDS_CH0_MODE_MASK |
		 IOMUXC_GPR2_LVDS_CH1_MODE_MASK);

	writel(reg, &iomux->gpr[2]);
}

static void do_enable_hdmi(struct display_info_t const *dev)
{
	disable_lvds(dev);
	imx_enable_hdmi_phy();
}

/* Add by qinzd 2016-10-26 */
static iomux_v3_cfg_t const backlight_pads[] = {
	MX6_PAD_SD1_DAT2__GPIO1_IO19 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_SD1_DAT3__GPIO1_IO21 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_CS2__GPIO6_IO15 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_NANDF_CS3__GPIO6_IO16 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void enable_lvds(struct display_info_t const *dev)
{
//	struct iomuxc *iomux = (struct iomuxc *)
//				IOMUXC_BASE_ADDR;
//	u32 reg = readl(&iomux->gpr[2]);
//	reg |= IOMUXC_GPR2_DATA_WIDTH_CH0_18BIT |
//	       IOMUXC_GPR2_DATA_WIDTH_CH1_18BIT;
//	writel(reg, &iomux->gpr[2]);

	/* Add by qinzd 2016-10-26 */
	imx_iomux_v3_setup_multiple_pads(backlight_pads, ARRAY_SIZE(backlight_pads));
	gpio_direction_output(IMX_GPIO_NR(1, 19), 1);
	gpio_direction_output(IMX_GPIO_NR(1, 21), 1);
	gpio_direction_output(IMX_GPIO_NR(6, 15), 1);
	gpio_direction_output(IMX_GPIO_NR(6, 16), 1);
}

void disable_lvds_gpio(void)
{
	gpio_direction_output(IMX_GPIO_NR(6, 15), 0);
	gpio_direction_output(IMX_GPIO_NR(6, 16), 0);
}

struct display_info_t const displays[] = {{
	.bus	= 1,
	.addr	= 0,
	.pixfmt	= IPU_PIX_FMT_RGB666,
	.detect	= NULL,
	.enable	= enable_lvds,
	.mode	= {
		.name           = "panel640x480d18",
		.refresh		= 70,
		.xres			= 640,
		.yres			= 480,
		.pixclock		= 26143,
		.left_margin	= 200,
		.right_margin	= 109,
		.upper_margin	= 15,
		.lower_margin	= 15,
		.hsync_len		= 80,
		.vsync_len		= 21,
		.sync			= 0,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
	.bus	= 1,
	.addr	= 0,
	.pixfmt	= IPU_PIX_FMT_RGB24,
	.detect	= NULL,
	.enable	= enable_lvds,
	.mode	= {
		.name           = "panel640x480d24",
		.refresh		= 70,
		.xres			= 640,
		.yres			= 480,
		.pixclock		= 26143,
		.left_margin	= 200,
		.right_margin	= 109,
		.upper_margin	= 15,
		.lower_margin	= 15,
		.hsync_len		= 80,
		.vsync_len		= 21,
		.sync			= 0,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
	.bus	= 1,
	.addr	= 0,
	.pixfmt	= IPU_PIX_FMT_RGB666,
	.detect	= NULL,
	.enable	= enable_lvds,
	.mode	= {
		.name           = "panel800x480d18",
		.refresh		= 70,
		.xres			= 800,
		.yres			= 480,
		.pixclock		= 26143,
		.left_margin	= 110,
		.right_margin	= 39,
		.upper_margin	= 15,
		.lower_margin	= 15,
		.hsync_len		= 80,
		.vsync_len		= 21,
		.sync			= 0,
		.vmode          = FB_VMODE_NONINTERLACED
} }, {
	.bus	= 1,
	.addr	= 0,
	.pixfmt = IPU_PIX_FMT_RGB24,
	.detect = NULL,
	.enable = enable_lvds,
	.mode	= {
		.name			= "panel800x480d24",
		.refresh		= 70,
		.xres			= 800,
		.yres			= 480,
		.pixclock		= 26143,
		.left_margin	= 110,
		.right_margin	= 39,
		.upper_margin	= 15,
		.lower_margin	= 15,
		.hsync_len		= 80,
		.vsync_len		= 21,
		.sync			= 0,
		.vmode			= FB_VMODE_NONINTERLACED
} }, {
	.bus	= 1,
	.addr	= 0,
	.pixfmt = IPU_PIX_FMT_RGB666,
	.detect = NULL,
	.enable = enable_lvds,
	.mode	= {
		.name			= "panel800x600d18",
		.refresh		= 59,
		.xres			= 800,
		.yres			= 600,
		.pixclock		= 26143,
		.left_margin	= 110,
		.right_margin	= 39,
		.upper_margin	= 5,
		.lower_margin	= 20,
		.hsync_len		= 80,
		.vsync_len		= 5,
		.sync			= 0,
		.vmode			= FB_VMODE_NONINTERLACED
} }, {
	.bus	= 1,
	.addr	= 0,
	.pixfmt = IPU_PIX_FMT_RGB24,
	.detect = NULL,
	.enable = enable_lvds,
	.mode	= {
		.name			= "panel800x600d24",
		.refresh		= 59,
		.xres			= 800,
		.yres			= 600,
		.pixclock		= 26143,
		.left_margin	= 110,
		.right_margin	= 39,
		.upper_margin	= 5,
		.lower_margin	= 20,
		.hsync_len		= 80,
		.vsync_len		= 5,
		.sync			= 0,
		.vmode			= FB_VMODE_NONINTERLACED
} }, {//leelin
	.bus	= 1,
	.addr	= 0,
	.pixfmt = IPU_PIX_FMT_RGB666,
	.detect = NULL,
	.enable = enable_lvds,
	.mode	= {
		.name			= "panel1024x600d18",
		.refresh		= 58,
		.xres			= 1024,
		.yres			= 600,
		.pixclock		= 20623,
		.left_margin	= 147,
		.right_margin	= 48,
		.upper_margin	= 6,	
		.lower_margin	= 20,
		.hsync_len		= 104,
		.vsync_len		= 6,
		.sync			= 0,
		.vmode			= FB_VMODE_NONINTERLACED
} }, {
	.bus	= 1,
	.addr	= 0,
	.pixfmt = IPU_PIX_FMT_RGB24,
	.detect = NULL,
	.enable = enable_lvds,
	.mode	= {
		.name			= "panel1024x600d24",
		.refresh		= 60,
		.xres			= 1024, //1024+320=1344
		.yres			= 600,	//600+35=635
		.pixclock		= 19528,
		.left_margin	= 220,	//220+40+60=320
		.right_margin	= 40,
		.upper_margin	= 18,	//18+7+10=35
		.lower_margin	= 7,
		.hsync_len		= 60,
		.vsync_len		= 10,
		.sync			= 0,
		.vmode			= FB_VMODE_NONINTERLACED
} }, {
	.bus	= 1,
	.addr	= 0,
	.pixfmt = IPU_PIX_FMT_RGB666,
	.detect = NULL,
	.enable = enable_lvds,
	.mode	= {
		.name			= "panel1024x768d18",
		.refresh		= 60,
		.xres			= 1024,
		.yres			= 768,
		.pixclock		= 15748,
		.left_margin	= 147,	
		.right_margin	= 48,
		.upper_margin	= 5,	
		.lower_margin	= 21,
		.hsync_len		= 104,
		.vsync_len		= 5,
		.sync			= 0,
		.vmode			= FB_VMODE_NONINTERLACED
} }, {
	.bus	= 1,
	.addr	= 0,
	.pixfmt = IPU_PIX_FMT_RGB24,
	.detect = NULL,
	.enable = enable_lvds,
	.mode	= {
		.name			= "panel1024x768d24",
		.refresh		= 60,
		.xres			= 1024,
		.yres			= 768,
		.pixclock		= 15748,
		.left_margin	= 147,	
		.right_margin	= 48,
		.upper_margin	= 5,	
		.lower_margin	= 21,
		.hsync_len		= 104,
		.vsync_len		= 5,
		.sync			= 0,
		.vmode			= FB_VMODE_NONINTERLACED
} }, {//lee
	.bus	= 1,
	.addr	= 0,
	.pixfmt = IPU_PIX_FMT_RGB666,
	.detect = NULL,
	.enable = enable_lvds,
	.mode	= {
		.name			= "panel1280x800d18",
		.refresh		= 60,
		.xres			= 1280,	//1280+400=1680
		.yres			= 800,	//800+31=831
		.pixclock		= 11784,
		.left_margin	= 200,	//220+72+128=400
		.right_margin	= 72,
		.upper_margin	= 22,	//22+3+6=31
		.lower_margin	= 10,
		.hsync_len		= 128,
		.vsync_len		= 10,
		.sync			= 0,
		.vmode			= FB_VMODE_NONINTERLACED
} }, {//lee
	.bus	= 1,
	.addr	= 0,
	.pixfmt = IPU_PIX_FMT_RGB24,
	.detect = NULL,
	.enable = enable_lvds,
	.mode	= {
		.name			= "panel1280x800d24",
		.refresh		= 60,
		.xres			= 1280,	//1280+400=1680
		.yres			= 800,	//800+42=842
		.pixclock		= 11784,
		.left_margin	= 200,	//200+72+128=400
		.right_margin	= 72,
		.upper_margin	= 22,	//22+10+10=42
		.lower_margin	= 10,
		.hsync_len		= 128,
		.vsync_len		= 10,
		.sync			= 0,
		.vmode			= FB_VMODE_NONINTERLACED
} }, {
	.bus	= 1,
	.addr	= 0,
	.pixfmt = IPU_PIX_FMT_RGB666,
	.detect = NULL,
	.enable = enable_lvds,
	.mode	= {
		.name			= "panel1366x768d18",
		.refresh		= 60,
		.xres			= 1366,
		.yres			= 768,
		.pixclock		= 13257,
		.left_margin	= 50,
		.right_margin	= 50,
		.upper_margin	= 9,
		.lower_margin	= 9,
		.hsync_len		= 94,
		.vsync_len		= 20,
		.sync			= 0,
		.vmode			= FB_VMODE_NONINTERLACED
} }, {
	.bus	= 1,
	.addr	= 0,
	.pixfmt = IPU_PIX_FMT_RGB24,
	.detect = NULL,
	.enable = enable_lvds,
	.mode	= {
		.name			= "panel1366x768d24",
		.refresh		= 60,
		.xres			= 1366,
		.yres			= 768,
		.pixclock		= 13257,
		.left_margin	= 50,
		.right_margin	= 50,
		.upper_margin	= 9,
		.lower_margin	= 9,
		.hsync_len		= 94,
		.vsync_len		= 20,
		.sync			= 0,
		.vmode			= FB_VMODE_NONINTERLACED
} }, {
	.bus	= 1,
	.addr	= 0,
	.pixfmt = IPU_PIX_FMT_RGB666,
	.detect = NULL,
	.enable = enable_lvds,
	.mode	= {
		.name			= "panel1920x1080d18",
		.refresh		= 60,
		.xres			= 1920,
		.yres			= 1080,
		.pixclock		= 13250,
		.left_margin	= 100,
		.right_margin	= 100,
		.upper_margin	= 10,
		.lower_margin	= 10,
		.hsync_len		= 80,
		.vsync_len		= 18,
		.sync			= 0,
		.vmode			= FB_VMODE_NONINTERLACED
} }, {
	.bus	= 1,
	.addr	= 0,
	.pixfmt = IPU_PIX_FMT_RGB24,
	.detect = NULL,
	.enable = enable_lvds,
	.mode	= {
		.name			= "panel1920x1080d24",
#if 1
		.refresh		= 60,
		.xres			= 1920,
		.yres			= 1080,
		.pixclock		= 13250,
		.left_margin	= 100,
		.right_margin	= 100,
		.upper_margin	= 10,
		.lower_margin	= 10,
		.hsync_len		= 80,
		.vsync_len		= 18,
		.sync			= 0,
#else
		.refresh		= 60,
		.xres			= 1920,
		.yres			= 1080,
		.pixclock		= 11560,
		.left_margin	= 328,
		.right_margin	= 128,
		.upper_margin	= 3,
		.lower_margin	= 32,
		.hsync_len		= 200,
		.vsync_len		= 5,
		.sync			= 0,
#endif
		.vmode			= FB_VMODE_NONINTERLACED
} } };

size_t display_count = ARRAY_SIZE(displays);

static void setup_display(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;
	int reg;

	/* Setup HSYNC, VSYNC, DISP_CLK for debugging purposes */
	imx_iomux_v3_setup_multiple_pads(di0_pads, ARRAY_SIZE(di0_pads));

	enable_ipu_clock();
	imx_setup_hdmi();

	/* Turn on LDB0, LDB1, IPU,IPU DI0 clocks */
	reg = readl(&mxc_ccm->CCGR3);
	reg |=  MXC_CCM_CCGR3_LDB_DI0_MASK | MXC_CCM_CCGR3_LDB_DI1_MASK;
	writel(reg, &mxc_ccm->CCGR3);

	/* set LDB0, LDB1 clk select to 011/011 */
	reg = readl(&mxc_ccm->cs2cdr);
	reg &= ~(MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK
		 | MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK);
	reg |= (3 << MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET)
	      | (3 << MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->cs2cdr);

	reg = readl(&mxc_ccm->cscmr2);
	reg |= MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV | MXC_CCM_CSCMR2_LDB_DI1_IPU_DIV;
	writel(reg, &mxc_ccm->cscmr2);

	reg = readl(&mxc_ccm->chsccdr);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0
		<< MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0
		<< MXC_CCM_CHSCCDR_IPU1_DI1_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->chsccdr);

	reg = IOMUXC_GPR2_BGREF_RRMODE_EXTERNAL_RES
	     | IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_LOW
	     | IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_LOW
	     | IOMUXC_GPR2_BIT_MAPPING_CH1_SPWG
	     | IOMUXC_GPR2_DATA_WIDTH_CH1_18BIT
	     | IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG
	     | IOMUXC_GPR2_DATA_WIDTH_CH0_18BIT
	     | IOMUXC_GPR2_LVDS_CH0_MODE_DISABLED
	     | IOMUXC_GPR2_LVDS_CH1_MODE_ENABLED_DI0;
	writel(reg, &iomux->gpr[2]);

	reg = readl(&iomux->gpr[3]);
	reg = (reg & ~(IOMUXC_GPR3_LVDS1_MUX_CTL_MASK
			| IOMUXC_GPR3_HDMI_MUX_CTL_MASK))
	    | (IOMUXC_GPR3_MUX_SRC_IPU1_DI0
	       << IOMUXC_GPR3_LVDS1_MUX_CTL_OFFSET);
	writel(reg, &iomux->gpr[3]);
}
#endif /* CONFIG_VIDEO_IPUV3 */

#ifdef CONFIG_UBOOT_LOGO_ENABLE_OVER_8BITS
#ifdef IPU_OUTPUT_MODE_LCD
static void ipu_iomux_config(void)
{
	iomux_v3_cfg_t display_pads[] = {
		MX6_PAD_DI0_DISP_CLK__IPU1_DI0_DISP_CLK | MUX_PAD_CTRL(NO_PAD_CTRL),
		MX6_PAD_DI0_PIN15__IPU1_DI0_PIN15 | MUX_PAD_CTRL(NO_PAD_CTRL), 	/* DE */
		MX6_PAD_DI0_PIN2__IPU1_DI0_PIN02 | MUX_PAD_CTRL(NO_PAD_CTRL),		/* HSync */
		MX6_PAD_DI0_PIN3__IPU1_DI0_PIN03 | MUX_PAD_CTRL(NO_PAD_CTRL),		/* VSync */
		MX6_PAD_DISP0_DAT0__IPU1_DISP0_DATA00 | MUX_PAD_CTRL(NO_PAD_CTRL),
		MX6_PAD_DISP0_DAT1__IPU1_DISP0_DATA01 | MUX_PAD_CTRL(NO_PAD_CTRL),
		MX6_PAD_DISP0_DAT2__IPU1_DISP0_DATA02 | MUX_PAD_CTRL(NO_PAD_CTRL),
		MX6_PAD_DISP0_DAT3__IPU1_DISP0_DATA03 | MUX_PAD_CTRL(NO_PAD_CTRL),
		MX6_PAD_DISP0_DAT4__IPU1_DISP0_DATA04 | MUX_PAD_CTRL(NO_PAD_CTRL),
		MX6_PAD_DISP0_DAT5__IPU1_DISP0_DATA05 | MUX_PAD_CTRL(NO_PAD_CTRL),
		MX6_PAD_DISP0_DAT6__IPU1_DISP0_DATA06 | MUX_PAD_CTRL(NO_PAD_CTRL),
		MX6_PAD_DISP0_DAT7__IPU1_DISP0_DATA07 | MUX_PAD_CTRL(NO_PAD_CTRL),
		MX6_PAD_DISP0_DAT8__IPU1_DISP0_DATA08 | MUX_PAD_CTRL(NO_PAD_CTRL),
		MX6_PAD_DISP0_DAT9__IPU1_DISP0_DATA09 | MUX_PAD_CTRL(NO_PAD_CTRL),
		MX6_PAD_DISP0_DAT10__IPU1_DISP0_DATA10 | MUX_PAD_CTRL(NO_PAD_CTRL),
		MX6_PAD_DISP0_DAT11__IPU1_DISP0_DATA11 | MUX_PAD_CTRL(NO_PAD_CTRL),
		MX6_PAD_DISP0_DAT12__IPU1_DISP0_DATA12 | MUX_PAD_CTRL(NO_PAD_CTRL),
		MX6_PAD_DISP0_DAT13__IPU1_DISP0_DATA13 | MUX_PAD_CTRL(NO_PAD_CTRL),
		MX6_PAD_DISP0_DAT14__IPU1_DISP0_DATA14 | MUX_PAD_CTRL(NO_PAD_CTRL),
		MX6_PAD_DISP0_DAT15__IPU1_DISP0_DATA15 | MUX_PAD_CTRL(NO_PAD_CTRL),
		MX6_PAD_DISP0_DAT16__IPU1_DISP0_DATA16 | MUX_PAD_CTRL(NO_PAD_CTRL),
		MX6_PAD_DISP0_DAT17__IPU1_DISP0_DATA17 | MUX_PAD_CTRL(NO_PAD_CTRL),
		MX6_PAD_DISP0_DAT18__IPU1_DISP0_DATA18 | MUX_PAD_CTRL(NO_PAD_CTRL),
		MX6_PAD_DISP0_DAT19__IPU1_DISP0_DATA19 | MUX_PAD_CTRL(NO_PAD_CTRL),
		MX6_PAD_DISP0_DAT20__IPU1_DISP0_DATA20 | MUX_PAD_CTRL(NO_PAD_CTRL),
		MX6_PAD_DISP0_DAT21__IPU1_DISP0_DATA21 | MUX_PAD_CTRL(NO_PAD_CTRL),
		MX6_PAD_DISP0_DAT22__IPU1_DISP0_DATA22 | MUX_PAD_CTRL(NO_PAD_CTRL),
		MX6_PAD_DISP0_DAT23__IPU1_DISP0_DATA23 | MUX_PAD_CTRL(NO_PAD_CTRL),
	};

	imx_iomux_v3_setup_multiple_pads(display_pads,
			ARRAY_SIZE(display_pads));
}
#endif

#ifdef IPU_OUTPUT_MODE_LVDS
static void setup_lvds_iomux(void)
{
	struct pwm_device pwm = {
		.pwm_id = 0,
		.pwmo_invert = 0,
	};

	imx_pwm_config(pwm, 25000, 50000);
	imx_pwm_enable(pwm);

	/* GPIO backlight */
	imx_iomux_v3_setup_pad(MX6_PAD_SD1_DAT3__PWM1_OUT | MUX_PAD_CTRL(NO_PAD_CTRL));
	/* LVDS panel CABC_EN */
	imx_iomux_v3_setup_pad(MX6_PAD_NANDF_CS2__GPIO6_IO15 | MUX_PAD_CTRL(NO_PAD_CTRL));
	imx_iomux_v3_setup_pad(MX6_PAD_NANDF_CS3__GPIO6_IO16 | MUX_PAD_CTRL(NO_PAD_CTRL));

	/*
	 * Set LVDS panel CABC_EN to low to disable
	 * CABC function. This function will turn backlight
	 * automatically according to display content, so
	 * simply disable it to get rid of annoying unstable
	 * backlight phenomena.
	 */
	gpio_direction_output(IMX_GPIO_NR(6, 15), 1);
	gpio_direction_output(IMX_GPIO_NR(6, 16), 1);
}
#endif

#ifdef IPU_OUTPUT_MODE_LCD
static void setup_lcd_iomux(void)
{
	unsigned int reg;
	struct pwm_device pwm = {
		.pwm_id = 0,
		.pwmo_invert = 0,
	};

	imx_pwm_config(pwm, 25000, 50000);
	imx_pwm_enable(pwm);

	/* LCD Power */
	imx_iomux_v3_setup_pad(MX6_PAD_ENET_TXD0__GPIO1_IO30 | MUX_PAD_CTRL(NO_PAD_CTRL));
	/* LCD reset */
	imx_iomux_v3_setup_pad(MX6_PAD_EIM_DA8__GPIO3_IO08 | MUX_PAD_CTRL(NO_PAD_CTRL));
	/* Backlight */
	imx_iomux_v3_setup_pad(MX6_PAD_SD1_DAT3__PWM1_OUT | MUX_PAD_CTRL(NO_PAD_CTRL));

	/* Set LCD Power to high. */
	gpio_direction_output(IMX_GPIO_NR(1, 30), 1);

	/* Set LCD reset to high. */
	gpio_direction_output(IMX_GPIO_NR(3, 8), 1);
}
#endif

#ifdef IPU_OUTPUT_MODE_HDMI
static void setup_hdmi_iomux(void)
{
	iomux_v3_cfg_t hdmi_i2c_pads[] = {
		MX6_PAD_KEY_COL3__HDMI_TX_DDC_SCL | MUX_PAD_CTRL(NO_PAD_CTRL),
		MX6_PAD_KEY_ROW3__HDMI_TX_DDC_SDA | MUX_PAD_CTRL(NO_PAD_CTRL),
	};

	imx_iomux_v3_setup_multiple_pads(hdmi_i2c_pads,
			ARRAY_SIZE(hdmi_i2c_pads));
}
#endif

#endif  //CONFIG_UBOOT_LOGO_ENABLE_OVER_8BITS

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

static void setup_fec(void)
{
	if (is_mx6dqp()) {
		int ret;

		/* select ENET MAC0 TX clock from PLL */
		imx_iomux_set_gpr_register(5, 9, 1, 1);
		ret = enable_fec_anatop_clock(0, ENET_125MHZ);
		if (ret)
		    printf("Error fec anatop clock settings!\n");
	}
}

int board_eth_init(bd_t *bis)
{
	setup_iomux_enet();
	setup_pcie();

	return cpu_eth_init(bis);
}

#ifdef CONFIG_USB_EHCI_MX6
#define USB_OTHERREGS_OFFSET	0x800
#define UCTRL_PWR_POL		(1 << 9)

static iomux_v3_cfg_t const usb_otg_pads[] = {
	MX6_PAD_EIM_D22__USB_OTG_PWR | MUX_PAD_CTRL(NO_PAD_CTRL),
//	MX6_PAD_ENET_RX_ER__USB_OTG_ID | MUX_PAD_CTRL(OTG_ID_PAD_CTRL),
	MX6_PAD_GPIO_1__USB_OTG_ID | MUX_PAD_CTRL(OTG_ID_PAD_CTRL),
};

//static iomux_v3_cfg_t const usb_hc1_pads[] = {
//	MX6_PAD_ENET_TXD1__GPIO1_IO29 | MUX_PAD_CTRL(NO_PAD_CTRL),
//};

static void setup_usb(void)
{
//	imx_iomux_v3_setup_multiple_pads(usb_otg_pads,
//					 ARRAY_SIZE(usb_otg_pads));

	/*
	 * set daisy chain for otg_pin_id on 6q.
	 * for 6dl, this bit is reserved
	 */
	imx_iomux_set_gpr_register(1, 13, 1, 0);

//	imx_iomux_v3_setup_multiple_pads(usb_hc1_pads,
//					 ARRAY_SIZE(usb_hc1_pads));
}

int board_ehci_hcd_init(int port)
{
	u32 *usbnc_usb_ctrl;

	if (port > 1)
		return -EINVAL;

	usbnc_usb_ctrl = (u32 *)(USB_BASE_ADDR + USB_OTHERREGS_OFFSET +
				 port * 4);

	setbits_le32(usbnc_usb_ctrl, UCTRL_PWR_POL);

	return 0;
}

int board_ehci_power(int port, int on)
{
	switch (port) {
	case 0:
		break;
	case 1:
//		if (on)
//			gpio_direction_output(IMX_GPIO_NR(1, 29), 1);
//		else
//			gpio_direction_output(IMX_GPIO_NR(1, 29), 0);
		break;
	default:
		printf("MXC USB port %d not yet supported\n", port);
		return -EINVAL;
	}

	return 0;
}
#endif

int board_early_init_f(void)
{
	setup_iomux_uart();
#if defined(CONFIG_VIDEO_IPUV3)
	setup_display();
#endif

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;
	set_LVDS_VCC(0);
#ifdef CONFIG_MXC_SPI
//	setup_spi();
#endif
	setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);

#ifdef CONFIG_EEPROM_GPIO_I2C4
	/* GPIO to I2C4  SCL*/
	imx_iomux_v3_setup_pad(MX6_PAD_ENET_TX_EN__GPIO1_IO28 | MUX_PAD_CTRL(NO_PAD_CTRL));
	/* GPIO to I2C4 SDA*/
	imx_iomux_v3_setup_pad(MX6_PAD_ENET_TXD1__GPIO1_IO29 | MUX_PAD_CTRL(NO_PAD_CTRL));
#else
	setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info2);
#endif
	eeprom_i2c_init();
#ifdef CONFIG_USB_EHCI_MX6
	setup_usb();
#endif

#if defined(CONFIG_MX6DL) && defined(CONFIG_MXC_EPDC)
	setup_epdc();
#endif

#ifdef CONFIG_CMD_SATA
	setup_sata();
#endif

#ifdef CONFIG_FEC_MXC
	setup_fec();
#endif
	set_LVDS_VCC(1);

	return 0;
}

int power_init_board(void)
{
	struct pmic *pfuze;
	unsigned int reg;
	int ret;

	pfuze = pfuze_common_init(I2C_PMIC);
	if (!pfuze)
		return -ENODEV;

	if (is_mx6dqp())
		ret = pfuze_mode_init(pfuze, APS_APS);
	else
		ret = pfuze_mode_init(pfuze, APS_PFM);

	if (ret < 0)
		return ret;
	/* VGEN3 and VGEN5 corrected on i.mx6qp board */
	if (!is_mx6dqp()) {
		/* Increase VGEN3 from 2.5 to 2.8V */
		pmic_reg_read(pfuze, PFUZE100_VGEN3VOL, &reg);
		reg &= ~LDO_VOL_MASK;
		reg |= LDOB_2_80V;
		pmic_reg_write(pfuze, PFUZE100_VGEN3VOL, reg);

		/* Increase VGEN5 from 2.8 to 3V */
		pmic_reg_read(pfuze, PFUZE100_VGEN5VOL, &reg);
		reg &= ~LDO_VOL_MASK;
		reg |= LDOB_3_00V;
		pmic_reg_write(pfuze, PFUZE100_VGEN5VOL, reg);
	}

	if (is_mx6dqp()) {
		/* set SW1C staby volatage 1.075V*/
		pmic_reg_read(pfuze, PFUZE100_SW1CSTBY, &reg);
		reg &= ~0x3f;
		reg |= 0x1f;
		pmic_reg_write(pfuze, PFUZE100_SW1CSTBY, reg);

		/* set SW1C/VDDSOC step ramp up time to from 16us to 4us/25mV */
		pmic_reg_read(pfuze, PFUZE100_SW1CCONF, &reg);
		reg &= ~0xc0;
		reg |= 0x40;
		pmic_reg_write(pfuze, PFUZE100_SW1CCONF, reg);

		/* set SW2/VDDARM staby volatage 0.975V*/
		pmic_reg_read(pfuze, PFUZE100_SW2STBY, &reg);
		reg &= ~0x3f;
		reg |= 0x17;
		pmic_reg_write(pfuze, PFUZE100_SW2STBY, reg);

		/* set SW2/VDDARM step ramp up time to from 16us to 4us/25mV */
		pmic_reg_read(pfuze, PFUZE100_SW2CONF, &reg);
		reg &= ~0xc0;
		reg |= 0x40;
		pmic_reg_write(pfuze, PFUZE100_SW2CONF, reg);
	} else {
		/* set SW1AB staby volatage 0.975V*/
		pmic_reg_read(pfuze, PFUZE100_SW1ABSTBY, &reg);
		reg &= ~0x3f;
		reg |= 0x1b;
		pmic_reg_write(pfuze, PFUZE100_SW1ABSTBY, reg);

		/* set SW1AB/VDDARM step ramp up time from 16us to 4us/25mV */
		pmic_reg_read(pfuze, PFUZE100_SW1ABCONF, &reg);
		reg &= ~0xc0;
		reg |= 0x40;
		pmic_reg_write(pfuze, PFUZE100_SW1ABCONF, reg);

		/* set SW1C staby volatage 0.975V*/
		pmic_reg_read(pfuze, PFUZE100_SW1CSTBY, &reg);
		reg &= ~0x3f;
		reg |= 0x1b;
		pmic_reg_write(pfuze, PFUZE100_SW1CSTBY, reg);

		/* set SW1C/VDDSOC step ramp up time to from 16us to 4us/25mV */
		pmic_reg_read(pfuze, PFUZE100_SW1CCONF, &reg);
		reg &= ~0xc0;
		reg |= 0x40;
		pmic_reg_write(pfuze, PFUZE100_SW1CCONF, reg);
	}

	return 0;
}

#ifdef CONFIG_LDO_BYPASS_CHECK
void ldo_mode_set(int ldo_bypass)
{
	unsigned int value;
	int is_400M;
	unsigned char vddarm;
	struct pmic *p = pmic_get("PFUZE100");

	if (!p) {
		printf("No PMIC found!\n");
		return;
	}

	/* increase VDDARM/VDDSOC to support 1.2G chip */
	if (check_1_2G()) {
		ldo_bypass = 0;	/* ldo_enable on 1.2G chip */
		printf("1.2G chip, increase VDDARM_IN/VDDSOC_IN\n");
		if (is_mx6dqp()) {
			/* increase VDDARM to 1.425V */
			pmic_reg_read(p, PFUZE100_SW2VOL, &value);
			value &= ~0x3f;
			value |= 0x29;
			pmic_reg_write(p, PFUZE100_SW2VOL, value);
		} else {
			/* increase VDDARM to 1.425V */
			pmic_reg_read(p, PFUZE100_SW1ABVOL, &value);
			value &= ~0x3f;
			value |= 0x2d;
			pmic_reg_write(p, PFUZE100_SW1ABVOL, value);
		}
		/* increase VDDSOC to 1.425V */
		pmic_reg_read(p, PFUZE100_SW1CVOL, &value);
		value &= ~0x3f;
		value |= 0x2d;
		pmic_reg_write(p, PFUZE100_SW1CVOL, value);
	}
	/* switch to ldo_bypass mode , boot on 800Mhz */
	if (ldo_bypass) {
		prep_anatop_bypass();
		if (is_mx6dqp()) {
			/* decrease VDDARM for 400Mhz DQP:1.1V*/
			pmic_reg_read(p, PFUZE100_SW2VOL, &value);
			value &= ~0x3f;
			value |= 0x1c;
			pmic_reg_write(p, PFUZE100_SW2VOL, value);
		} else {
			/* decrease VDDARM for 400Mhz DQ:1.1V, DL:1.275V */
			pmic_reg_read(p, PFUZE100_SW1ABVOL, &value);
			value &= ~0x3f;
			if (is_cpu_type(MXC_CPU_MX6DL))
				value |= 0x27;
			else
				value |= 0x20;

			pmic_reg_write(p, PFUZE100_SW1ABVOL, value);
		}
		/* increase VDDSOC to 1.3V */
		pmic_reg_read(p, PFUZE100_SW1CVOL, &value);
		value &= ~0x3f;
		value |= 0x28;
		pmic_reg_write(p, PFUZE100_SW1CVOL, value);

		/*
		 * MX6Q/DQP:
		 * VDDARM:1.15V@800M; VDDSOC:1.175V@800M
		 * VDDARM:0.975V@400M; VDDSOC:1.175V@400M
		 * MX6DL:
		 * VDDARM:1.175V@800M; VDDSOC:1.175V@800M
		 * VDDARM:1.075V@400M; VDDSOC:1.175V@400M
		 */
		is_400M = set_anatop_bypass(2);
		if (is_mx6dqp()) {
			pmic_reg_read(p, PFUZE100_SW2VOL, &value);
			value &= ~0x3f;
			if (is_400M)
				value |= 0x17;
			else
				value |= 0x1e;
			pmic_reg_write(p, PFUZE100_SW2VOL, value);
		}

		if (is_400M) {
			if (is_cpu_type(MXC_CPU_MX6DL))
				vddarm = 0x1f;
			else
				vddarm = 0x1b;
		} else {
			if (is_cpu_type(MXC_CPU_MX6DL))
				vddarm = 0x23;
			else
				vddarm = 0x22;
		}
		pmic_reg_read(p, PFUZE100_SW1ABVOL, &value);
		value &= ~0x3f;
		value |= vddarm;
		pmic_reg_write(p, PFUZE100_SW1ABVOL, value);

		/* decrease VDDSOC to 1.175V */
		pmic_reg_read(p, PFUZE100_SW1CVOL, &value);
		value &= ~0x3f;
		value |= 0x23;
		pmic_reg_write(p, PFUZE100_SW1CVOL, value);

		finish_anatop_bypass();
		printf("switch to ldo_bypass mode!\n");
	}
}
#endif

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	/* 4 bit bus width */
	{"sd2",	 MAKE_CFGVAL(0x40, 0x28, 0x00, 0x00)},
	{"sd3",	 MAKE_CFGVAL(0x40, 0x30, 0x00, 0x00)},
	/* 8 bit bus width */
	{"emmc", MAKE_CFGVAL(0x60, 0x58, 0x00, 0x00)},
	{NULL,	 0},
};
#endif

static void customer_lvds(void)
{
	unsigned int reg=0;
	uchar color_depth=eeprom_i2c_get_color_depth();
	uchar display_type=eeprom_i2c_get_type();
	uchar display_edid=eeprom_i2c_get_EDID();
#if 1
	if(display_type== 1)
	{
		imx_iomux_set_gpr_register(3, 8, 2, 1);
		//if (display_edid == RESOLUTION_1920X1080)
		//	reg |= (1 << 10);
		if (color_depth == 24)
		{
			reg |= (1 << 7);
			if (display_edid == RESOLUTION_1920X1080)
				reg |= (1 << 5);
		}
		reg |= (3 << 2);
		if (display_edid == RESOLUTION_1920X1080)
		{
			reg |= (1 << 4);
			reg |= (3 << 0);
		}
		writel(reg, IOMUXC_BASE_ADDR + 0x8);
	}
#else
#if (LVDS_PORT == 0)
		if(IPU == 1)
		{
			if(DI == 0)
				imx_iomux_set_gpr_register(3, 6, 2, 0);
			else if(DI == 1)
				imx_iomux_set_gpr_register(3, 6, 2, 1);
		}
	
		if(IPU == 2)
		{
			if(DI == 0)
				imx_iomux_set_gpr_register(3, 6, 2, 2);
			else if(DI == 1)
				imx_iomux_set_gpr_register(3, 6, 2, 3);
		}
#endif
	
#if (LVDS_PORT == 1)
		if(IPU == 1)
		{
			if(DI == 0)
				imx_iomux_set_gpr_register(3, 8, 2, 0);
			else if(DI == 1)
				imx_iomux_set_gpr_register(3, 8, 2, 1);
		}
	
		if(IPU == 2)
		{
			if(DI == 0)
				imx_iomux_set_gpr_register(3, 8, 2, 2);
			else if(DI == 1)
				imx_iomux_set_gpr_register(3, 8, 2, 3);
		}
#endif
	
		reg = 0;
		if (DI == 0)
			reg |= (DISPLAY_VSYNC_POLARITY << 9);
		else if(DI == 1)
			reg |= (DISPLAY_VSYNC_POLARITY << 10);
	
#if (LVDS_PORT == 0)
#if (DISPLAY_IF_BPP == 24)
		reg |= (1 << 5);
#ifdef LVDS_SPLIT_MODE
		reg |= (1 << 7);
#endif
#endif
	
		if (DI == 0)
			reg |= (1 << 0);
		else if(DI == 1)
			reg |= (3 << 0);
	
#ifdef LVDS_SPLIT_MODE
		reg |= (1 << 4);
		if (DI == 0)
			reg |= (1 << 2);
		else if(DI == 1)
			reg |= (3 << 2);
#endif
#endif
	
#if (LVDS_PORT == 1)
#if (DISPLAY_IF_BPP == 24)
		reg |= (1 << 7);
#ifdef LVDS_SPLIT_MODE
		reg |= (1 << 5);
#endif
#endif
	
		if (DI == 0)
			reg |= (1 << 2);
		else if(DI == 1)
			reg |= (3 << 2);
#ifdef LVDS_SPLIT_MODE
		reg |= (1 << 4);
		if (DI == 0)
			reg |= (1 << 0);
		else if(DI == 1)
			reg |= (3 << 0);
#endif
#endif
	
		writel(reg, IOMUXC_BASE_ADDR + 0x8);  //Set LDB_CTRL

#endif
	return;
}

int board_late_init(void)
{
	unsigned char * pData;
	u32 clocksource=0;
#ifdef CONFIG_UBOOT_LOGO_ENABLE_OVER_8BITS	//for 16bpp or 32bpp logo
	unsigned int size = DISPLAY_WIDTH * DISPLAY_HEIGHT * (DISPLAY_BPP / 8);
	unsigned int start, count;
	int i, bmpReady = 0;
#if 1
	int mmc_dev = mmc_get_env_devno();
	struct mmc *mmc = find_mmc_device(mmc_dev);
#endif

	pData = (unsigned char *)CONFIG_FB_BASE;
#if 0
	if (mmc)	{
		if (mmc_init(mmc) == 0) {
			start = ALIGN(UBOOT_LOGO_BMP_ADDR, mmc->read_bl_len) / mmc->read_bl_len;
			count = ALIGN(size, mmc->read_bl_len) / mmc->read_bl_len;
			mmc->block_dev.block_read(mmc_dev, start, count, pData);
			bmpReady = 1;
		}
	}
#endif

	if (bmpReady == 0) {
		// Fill RGB frame buffer
		// Red
		for (i = 0; i < (DISPLAY_WIDTH * DISPLAY_HEIGHT * (DISPLAY_BPP / 8) / 3); i += (DISPLAY_BPP / 8)) {
#if (DISPLAY_BPP == 16)
			pData[i + 0] = 0x00;
			pData[i + 1] = 0xF8;
#else
			pData[i + 0] = 0x00;
			pData[i + 1] = 0x00;
			pData[i + 2] = 0xFF;
			pData[i + 3] = 0x00;
#endif
		}

		// Green
		for (; i < (DISPLAY_WIDTH * DISPLAY_HEIGHT * (DISPLAY_BPP / 8) / 3) * 2; i += (DISPLAY_BPP / 8)) {
#if (DISPLAY_BPP == 16)
			pData[i + 0] = 0xE0;
			pData[i + 1] = 0x07;
#else
			pData[i + 0] = 0x00;
			pData[i + 1] = 0xFF;
			pData[i + 2] = 0x00;
			pData[i + 3] = 0x00;
#endif
		}

		// Blue
		for (; i < DISPLAY_WIDTH * DISPLAY_HEIGHT * (DISPLAY_BPP / 8); i += (DISPLAY_BPP / 8)) {
#if (DISPLAY_BPP == 16)
			pData[i + 0] = 0x1F;
			pData[i + 1] = 0x00;
#else
			pData[i + 0] = 0xFF;
			pData[i + 1] = 0x00;
			pData[i + 2] = 0x00;
			pData[i + 3] = 0x00;
#endif
		}
	}
#ifndef CONFIG_SYS_DCACHE_OFF
	flush_dcache_range((u32)pData, (u32)(pData + DISPLAY_WIDTH * DISPLAY_HEIGHT * (DISPLAY_BPP / 8)));
#endif

#ifdef IPU_OUTPUT_MODE_LVDS
	setup_lvds_iomux();
#endif

#ifdef IPU_OUTPUT_MODE_LCD
	ipu_iomux_config();
	setup_lcd_iomux();
#endif

#ifdef IPU_OUTPUT_MODE_HDMI
	setup_hdmi_iomux();
#endif

	ipu_display_setup(IPU_NUM, DI_NUM);
#else	//test
	customer_lvds();
	pData = (unsigned char *)CONFIG_FB_BASE;
	clocksource=MXC_IPU1_LVDS_DI1_CLK;
	switch(eeprom_i2c_get_EDID())
	{
		//setenv("bootargs","console=ttymxc0,115200 init=/init video=mxcfb0:dev=ldb,800x480M@70,if=RGB666,bpp=32 video=mxcfb1:off video=mxcfb2:off fbmem=40M fb0base=0x27b00000 vmalloc=400M androidboot.console=ttymxc0 androidboot.hardware=freescale mem=1024M\0");
		case RESOLUTION_640X480:
			display_clk_config(clocksource, 38000000);
			set_kernel_env(640,480);
			copy_bmp_screen((char *)pData,(char *)fsl_bmp_reversed_600x400,640,480);
			break;
		default:
		case RESOLUTION_800X480:
			display_clk_config(clocksource, 38000000);
			set_kernel_env(800,480);
			copy_bmp_screen((char *)pData,(char *)fsl_bmp_reversed_600x400,800,480);
			break;
		case RESOLUTION_800X600:
			display_clk_config(clocksource, 38000000);
			set_kernel_env(800,600);
			copy_bmp_screen((char *)pData,(char *)fsl_bmp_reversed_600x400,800,600);
			break;
		case RESOLUTION_1024X600:
			//display_clk_config(clocksource, 51206400);
			display_clk_config(clocksource, 47000000);
			set_kernel_env(1024,600);
			copy_bmp_screen((char *)pData,(char *)fsl_bmp_reversed_600x400,1024,600);
			break;
		case RESOLUTION_1024X768:
			display_clk_config(clocksource, 64000000);
			set_kernel_env(1024,768);
			copy_bmp_screen((char *)pData,(char *)fsl_bmp_reversed_600x400,1024,768);
			break;
		case RESOLUTION_1280X800:
			//display_clk_config(clocksource, 65000000);
			display_clk_config(clocksource, 80000000);
			set_kernel_env(1280,800);
			copy_bmp_screen((char *)pData,(char *)fsl_bmp_reversed_600x400,1280,800);
			break;
		case RESOLUTION_1366X768:
			display_clk_config(clocksource, 74000000);
			set_kernel_env(1366,768);
			copy_bmp_screen((char *)pData,(char *)fsl_bmp_reversed_600x400,1366,768);
			break;
		case RESOLUTION_1920X1080:
			copy_bmp_screen((char *)pData,(char *)fsl_bmp_reversed_600x400,1920,1080);
			//display_split_clk_config(MXC_IPU1_LVDS_DI1_CLK, 83000000);
			display_split_clk_config(MXC_IPU1_LVDS_DI0_CLK, 65000000);
			set_kernel_env(1920,1080);
			break;
	}
//	pData = (unsigned char *)CONFIG_FB_BASE;
//	memcpy(pData,fsl_bmp_reversed_600x400,fsl_bmp_reversed_600x400_size);
	video_display_bitmap((ulong)pData,0,0);
#endif

#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	setenv("board_name", "SABRESD");

	if (is_mx6dqp())
		setenv("board_rev", "MX6QP");
	else if (is_cpu_type(MXC_CPU_MX6Q) || is_cpu_type(MXC_CPU_MX6D))
		setenv("board_rev", "MX6Q");
	else if (is_cpu_type(MXC_CPU_MX6DL) || is_cpu_type(MXC_CPU_MX6SOLO))
		setenv("board_rev", "MX6DL");
#endif

#ifdef CONFIG_ENV_IS_IN_MMC
	board_late_mmc_env_init();
#endif

	return 0;
}

int checkboard(void)
{
	puts("Board: MX6-SabreSD\n");
	return 0;
}

#ifdef CONFIG_FSL_FASTBOOT

void board_fastboot_setup(void)
{
	switch (get_boot_device()) {
#if defined(CONFIG_FASTBOOT_STORAGE_SATA)
	case SATA_BOOT:
		if (!getenv("fastboot_dev"))
			setenv("fastboot_dev", "sata");
		if (!getenv("bootcmd"))
			setenv("bootcmd", "boota sata");
		break;
#endif /*CONFIG_FASTBOOT_STORAGE_SATA*/
#if defined(CONFIG_FASTBOOT_STORAGE_MMC)
	case SD2_BOOT:
	case MMC2_BOOT:
	    if (!getenv("fastboot_dev"))
			setenv("fastboot_dev", "mmc0");
	    if (!getenv("bootcmd"))
			setenv("bootcmd", "boota mmc0");
	    break;
	case SD3_BOOT:
	case MMC3_BOOT:
	    if (!getenv("fastboot_dev"))
			setenv("fastboot_dev", "mmc1");
	    if (!getenv("bootcmd"))
			setenv("bootcmd", "boota mmc1");
	    break;
	case MMC4_BOOT:
	    if (!getenv("fastboot_dev"))
			setenv("fastboot_dev", "mmc2");
	    if (!getenv("bootcmd"))
			setenv("bootcmd", "boota mmc2");
	    break;
#endif /*CONFIG_FASTBOOT_STORAGE_MMC*/
	default:
		printf("unsupported boot devices\n");
		break;
	}

}

#ifdef CONFIG_ANDROID_RECOVERY

#define GPIO_VOL_DN_KEY 	IMX_GPIO_NR(5, 20)
//#define GPIO_VOL_DN_KEY IMX_GPIO_NR(1, 5)
iomux_v3_cfg_t const recovery_key_pads[] = {
//	(MX6_PAD_GPIO_5__GPIO1_IO05 | MUX_PAD_CTRL(NO_PAD_CTRL)),
	(MX6_PAD_CSI0_DATA_EN__GPIO5_IO20 | MUX_PAD_CTRL(NO_PAD_CTRL)),
};

int check_recovery_cmd_file(void)
{
    int button_pressed = 0;
    int recovery_mode = 0;

#ifdef CONFIG_SBC7112
	return recovery_mode || button_pressed;
#endif
    recovery_mode = recovery_check_and_clean_flag();

    /* Check Recovery Combo Button press or not. */
	imx_iomux_v3_setup_multiple_pads(recovery_key_pads,
			ARRAY_SIZE(recovery_key_pads));

    gpio_direction_input(GPIO_VOL_DN_KEY);

    if (gpio_get_value(GPIO_VOL_DN_KEY) == 0) { /* VOL_DN key is low assert */
		button_pressed = 1;
		printf("Recovery key pressed\n");
    }

    return recovery_mode || button_pressed;
}

void board_recovery_setup(void)
{
	int bootdev = get_boot_device();

	switch (bootdev) {
#if defined(CONFIG_FASTBOOT_STORAGE_SATA)
	case SATA_BOOT:
		if (!getenv("bootcmd_android_recovery"))
			setenv("bootcmd_android_recovery",
				"boota sata recovery");
		break;
#endif /*CONFIG_FASTBOOT_STORAGE_SATA*/
#if defined(CONFIG_FASTBOOT_STORAGE_MMC)
	case SD2_BOOT:
	case MMC2_BOOT:
		if (!getenv("bootcmd_android_recovery"))
			setenv("bootcmd_android_recovery",
				"boota mmc0 recovery");
		break;
	case SD3_BOOT:
	case MMC3_BOOT:
		if (!getenv("bootcmd_android_recovery"))
			setenv("bootcmd_android_recovery",
				"boota mmc1 recovery");
		break;
	case MMC4_BOOT:
		if (!getenv("bootcmd_android_recovery"))
			setenv("bootcmd_android_recovery",
				"boota mmc2 recovery");
		break;
#endif /*CONFIG_FASTBOOT_STORAGE_MMC*/
	default:
		printf("Unsupported bootup device for recovery: dev: %d\n",
			bootdev);
		return;
	}

	printf("setup env for recovery..\n");
	setenv("bootcmd", "run bootcmd_android_recovery");
}

#endif /*CONFIG_ANDROID_RECOVERY*/

#endif /*CONFIG_FSL_FASTBOOT*/


#ifdef CONFIG_SPL_BUILD
#include <spl.h>
#include <libfdt.h>

const struct mx6dq_iomux_ddr_regs mx6_ddr_ioregs = {
	.dram_sdclk_0 =  0x00020030,
	.dram_sdclk_1 =  0x00020030,
	.dram_cas =  0x00020030,
	.dram_ras =  0x00020030,
	.dram_reset =  0x00020030,
	.dram_sdcke0 =  0x00003000,
	.dram_sdcke1 =  0x00003000,
	.dram_sdba2 =  0x00000000,
	.dram_sdodt0 =  0x00003030,
	.dram_sdodt1 =  0x00003030,
	.dram_sdqs0 =  0x00000030,
	.dram_sdqs1 =  0x00000030,
	.dram_sdqs2 =  0x00000030,
	.dram_sdqs3 =  0x00000030,
	.dram_sdqs4 =  0x00000030,
	.dram_sdqs5 =  0x00000030,
	.dram_sdqs6 =  0x00000030,
	.dram_sdqs7 =  0x00000030,
	.dram_dqm0 =  0x00020030,
	.dram_dqm1 =  0x00020030,
	.dram_dqm2 =  0x00020030,
	.dram_dqm3 =  0x00020030,
	.dram_dqm4 =  0x00020030,
	.dram_dqm5 =  0x00020030,
	.dram_dqm6 =  0x00020030,
	.dram_dqm7 =  0x00020030,
};

const struct mx6dq_iomux_ddr_regs mx6dqp_ddr_ioregs = {
	.dram_sdclk_0 =  0x00000030,
	.dram_sdclk_1 =  0x00000030,
	.dram_cas =  0x00000030,
	.dram_ras =  0x00000030,
	.dram_reset =  0x00000030,
	.dram_sdcke0 =  0x00003000,
	.dram_sdcke1 =  0x00003000,
	.dram_sdba2 =  0x00000000,
	.dram_sdodt0 =  0x00003030,
	.dram_sdodt1 =  0x00003030,
	.dram_sdqs0 =  0x00000030,
	.dram_sdqs1 =  0x00000030,
	.dram_sdqs2 =  0x00000030,
	.dram_sdqs3 =  0x00000030,
	.dram_sdqs4 =  0x00000030,
	.dram_sdqs5 =  0x00000030,
	.dram_sdqs6 =  0x00000030,
	.dram_sdqs7 =  0x00000030,
	.dram_dqm0 =  0x00000030,
	.dram_dqm1 =  0x00000030,
	.dram_dqm2 =  0x00000030,
	.dram_dqm3 =  0x00000030,
	.dram_dqm4 =  0x00000030,
	.dram_dqm5 =  0x00000030,
	.dram_dqm6 =  0x00000030,
	.dram_dqm7 =  0x00000030,
};

const struct mx6dq_iomux_grp_regs mx6_grp_ioregs = {
	.grp_ddr_type =  0x000C0000,
	.grp_ddrmode_ctl =  0x00020000,
	.grp_ddrpke =  0x00000000,
	.grp_addds =  0x00000030,
	.grp_ctlds =  0x00000030,
	.grp_ddrmode =  0x00020000,
	.grp_b0ds =  0x00000030,
	.grp_b1ds =  0x00000030,
	.grp_b2ds =  0x00000030,
	.grp_b3ds =  0x00000030,
	.grp_b4ds =  0x00000030,
	.grp_b5ds =  0x00000030,
	.grp_b6ds =  0x00000030,
	.grp_b7ds =  0x00000030,
};

const struct mx6_mmdc_calibration mx6_mmcd_calib = {
	.p0_mpwldectrl0 =  0x001F001F,
	.p0_mpwldectrl1 =  0x001F001F,
	.p1_mpwldectrl0 =  0x00440044,
	.p1_mpwldectrl1 =  0x00440044,
	.p0_mpdgctrl0 =  0x434B0350,
	.p0_mpdgctrl1 =  0x034C0359,
	.p1_mpdgctrl0 =  0x434B0350,
	.p1_mpdgctrl1 =  0x03650348,
	.p0_mprddlctl =  0x4436383B,
	.p1_mprddlctl =  0x39393341,
	.p0_mpwrdlctl =  0x35373933,
	.p1_mpwrdlctl =  0x48254A36,
};

const struct mx6_mmdc_calibration mx6dqp_mmcd_calib = {
	.p0_mpwldectrl0 =  0x001B001E,
	.p0_mpwldectrl1 =  0x002E0029,
	.p1_mpwldectrl0 =  0x001B002A,
	.p1_mpwldectrl1 =  0x0019002C,
	.p0_mpdgctrl0 =  0x43240334,
	.p0_mpdgctrl1 =  0x0324031A,
	.p1_mpdgctrl0 =  0x43340344,
	.p1_mpdgctrl1 =  0x03280276,
	.p0_mprddlctl =  0x44383A3E,
	.p1_mprddlctl =  0x3C3C3846,
	.p0_mpwrdlctl =  0x2E303230,
	.p1_mpwrdlctl =  0x38283E34,
};

/* MT41K128M16JT-125 */
static struct mx6_ddr3_cfg mem_ddr = {
	.mem_speed = 1600,
	.density = 2,
	.width = 16,
	.banks = 8,
	.rowaddr = 14,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1375,
	.trcmin = 4875,
	.trasmin = 3500,
};

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0x00C03F3F, &ccm->CCGR0);
	writel(0x0030FC03, &ccm->CCGR1);
	writel(0x0FFFC000, &ccm->CCGR2);
	writel(0x3FF00000, &ccm->CCGR3);
	writel(0x00FFF300, &ccm->CCGR4);
	writel(0x0F0000C3, &ccm->CCGR5);
	writel(0x000003FF, &ccm->CCGR6);
}

static void gpr_init(void)
{
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;

	/* enable AXI cache for VDOA/VPU/IPU */
	writel(0xF00000CF, &iomux->gpr[4]);
	if (is_mx6dqp()) {
		/* set IPU AXI-id1 Qos=0x1 AXI-id0/2/3 Qos=0x7 */
		writel(0x007F007F, &iomux->gpr[6]);
		writel(0x007F007F, &iomux->gpr[7]);
	} else {
		/* set IPU AXI-id0 Qos=0xf(bypass) AXI-id1 Qos=0x7 */
		writel(0x007F007F, &iomux->gpr[6]);
		writel(0x007F007F, &iomux->gpr[7]);
	}
}

/*
 * This section requires the differentiation between iMX6 Sabre boards, but
 * for now, it will configure only for the mx6q variant.
 */
static void spl_dram_init(void)
{
	struct mx6_ddr_sysinfo sysinfo = {
		/* width of data bus:0=16,1=32,2=64 */
		.dsize = 2,
		/* config for full 4GB range so that get_mem_size() works */
		.cs_density = 32, /* 32Gb per CS */
		/* single chip select */
		.ncs = 1,
		.cs1_mirror = 0,
		.rtt_wr = 1 /*DDR3_RTT_60_OHM*/,	/* RTT_Wr = RZQ/4 */
		.rtt_nom = 1 /*DDR3_RTT_60_OHM*/,	/* RTT_Nom = RZQ/4 */
		.walat = 1,	/* Write additional latency */
		.ralat = 5,	/* Read additional latency */
		.mif3_mode = 3,	/* Command prediction working mode */
		.bi_on = 1,	/* Bank interleaving enabled */
		.sde_to_rst = 0x10,	/* 14 cycles, 200us (JEDEC default) */
		.rst_to_cke = 0x23,	/* 33 cycles, 500us (JEDEC default) */
		.ddr_type = DDR_TYPE_DDR3,
	};

	if (is_mx6dqp()) {
		mx6dq_dram_iocfg(64, &mx6dqp_ddr_ioregs, &mx6_grp_ioregs);
		mx6_dram_cfg(&sysinfo, &mx6dqp_mmcd_calib, &mem_ddr);
	} else {
		mx6dq_dram_iocfg(64, &mx6_ddr_ioregs, &mx6_grp_ioregs);
		mx6_dram_cfg(&sysinfo, &mx6_mmcd_calib, &mem_ddr);
	}
}

void board_init_f(ulong dummy)
{
	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	ccgr_init();
	gpr_init();

	/* iomux and setup of i2c */
	board_early_init_f();

	/* setup GP timer */
	timer_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	/* DDR initialization */
	spl_dram_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* load/boot image from boot device */
	board_init_r(NULL, 0);
}
#endif
