/*
 *
 */
#ifndef __EEPROM_H__
#define __EEPROM_H__

#define EEPROM_I2C_BUS					2
#define EEPROM_I2C_ADDRESS				0x50
#define EEPROM_MAX_SIZE 				64
#define EEPROM_ACCESSABLE_SIZE			64
#define EEPROM_ADDRESS_LENGTH			1
#define MMC_DEVICE_BUS					1

#include <linux/string.h>
#include <i2c.h>
#include "eeprom_display_info.h"

struct eeprom_data_analysis
{
	u16 version;//[0xfe]+256*[0xff]
	u8 mac1[8];//0x01 0x06 00:11:22:33:44:55
	u8 mac2[8];//0x02 0x06 22:33:44:55:66:77
	u8 softid[8];//0x03 0x06 0001~9999(4 digits in tow byte) 00000001~99999999(8 digits in 4 byte)
	u8 backlight[6];//0x10 0x04 0x00 0x10 0x00 00(pwm polarity 0,1,pwm min x/256,pwm frequency [0]+256*[x])
	//	u8 display[6];//0x10 0x04 0x02 0x12 0x3c 0x01
	//	AT24c02_eeprom.data.display[0]	must be 0x10
	//	AT24c02_eeprom.data.display[1]	data lenght 0x04
	//	AT24c02_eeprom.data.display[2]	EDID
	//	AT24c02_eeprom.data.display[3]	18(0x12) or 24(0x18) bits
	//	AT24c02_eeprom.data.display[4]	fresh rate default : 60
	//	AT24c02_eeprom.data.display[5]	0x01:lvds, 0x02:hdmi, 0x03:RGB
	u8 display[6];//0x10 0x04 0x02 0x12 0x3c 0x01
	u8 logo[3];//0x11 0x01 0x01
 };

struct eeprom_info
{
	u16 accessable_size;						// Max:64				 accessable byte
	u16 max_size;								// eeprom max byte
	u8 i2c_bus;									// eeprom work at i2c bus number
	u8 device_addr;								// eeprom device address
	u8 address_length;							// eeprom address register length
	u8 mmc_bus;									// load data from mmc bus number
	unsigned char content[EEPROM_ACCESSABLE_SIZE];		// content of eeprom from address 0 to eeprom_info.size
	struct eeprom_data_analysis data;			// parse data of eeprom_info.content
	int (*init)(void); 							// init struct eeprom_info
	//unsigned int (* read)(unsigned int addr, int alen, uint8_t *buf, int len);		// read data from eeprom
	//unsigned int (* write)(unsigned int addr, int alen, uint8_t *buf, int len); 	// write data to eeprom
	int (* parse_data)(void);					// synthesis content to struct eeprom_data
	int (* synthesis_data)(void);				// parse struct eeprom_data to content
	int (*load_config)(void);					// load data from eMMC or other device

	u16 size;									// eeprom byte
	u16 version;								//[0xfe]+256*[0xff]
	int show_pass_logo;
	uchar mbootdelay;
	uchar mHalt;
	unsigned char mIpaddress[16];
	unsigned char mServerip[16];
};

unsigned int eeprom_i2c_read(unsigned int addr, int alen, uint8_t *buffer, int len);
unsigned int eeprom_i2c_write(unsigned int addr, int alen, uint8_t *buffer, int len);

unsigned char eeprom_i2c_get_type(void);
unsigned char eeprom_i2c_get_EDID(void);
unsigned char eeprom_i2c_get_color_depth(void);
unsigned char eeprom_i2c_pass_logo(void);
void set_kernel_env(int width, int height);

int eeprom_i2c_parse_data(void);
int eeprom_i2c_synthesis_data(void);
int Load_config_from_mmc(void);
int eeprom_i2c_init(void);

#endif // __EEPROM_H__
