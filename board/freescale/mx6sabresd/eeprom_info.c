/*
 *
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

#include "eeprom_info.h"

extern struct mmc *find_mmc_device(int dev_num);
extern int mmc_init(struct mmc *mmc);
extern int fat_register_device(block_dev_desc_t *dev_desc, int part_no);
extern int file_fat_read(const char *filename, void *buffer, int maxsize);
extern void puts(const char *str);
extern void udelay(unsigned long usec);

struct eeprom_info AT24c02_eeprom;
#ifdef CONFIG_EEPROM_GPIO_I2C4
#define GPIO_I2C_SCL	IMX_GPIO_NR(1, 28)
#define GPIO_I2C_SDA	IMX_GPIO_NR(1, 29)
/*	gpio i2c speed 
	(5) 100k, 
	(50) 10k
 */
#define I2C_DELAY_TIME		50
#define I2C_DELAY			udelay(I2C_DELAY_TIME)

static int i2c_gpio_sda_get(void)
{
	return gpio_get_value(GPIO_I2C_SDA);
}
#if 0
static void i2c_gpio_sda_input(void)
{
	gpio_direction_input(GPIO_I2C_SDA);
}
#endif
static void i2c_gpio_sda_set(int bit)
{
	if (bit) {
		gpio_direction_input(GPIO_I2C_SDA);
	} else {
		gpio_direction_output(GPIO_I2C_SDA,bit);
	}
}

static void i2c_gpio_sda_wset(int bit)
{
	gpio_direction_output(GPIO_I2C_SDA,bit);
}

static void i2c_gpio_scl_set(int bit)
{
	gpio_direction_output(GPIO_I2C_SCL,bit);
}

static void i2c_gpio_write_bit(uchar bit)
{
	i2c_gpio_scl_set(0);
	I2C_DELAY;
	i2c_gpio_sda_set(bit);
	I2C_DELAY;
	i2c_gpio_scl_set(1);
	I2C_DELAY;
	I2C_DELAY;
}

static void i2c_gpio_wwrite_bit(uchar bit)
{
	i2c_gpio_scl_set(0);
	I2C_DELAY;
	i2c_gpio_sda_wset(bit);
	I2C_DELAY;
	i2c_gpio_scl_set(1);
	I2C_DELAY;
	I2C_DELAY;
}

static int i2c_gpio_check_nack(void)
{
	int count;
	int value;

	i2c_gpio_scl_set(1);
	//I2C_DELAY;
	count=0;
	while(count<=(I2C_DELAY_TIME*2))
	{
		count++;
		value = i2c_gpio_sda_get();
		if(value==0)break;
		udelay(1);
	}
	I2C_DELAY;
	i2c_gpio_scl_set(0);
	I2C_DELAY;
	I2C_DELAY;
	return value;
}

static int i2c_gpio_read_bit(void)
{
	int value;

	i2c_gpio_scl_set(1);
	I2C_DELAY;
	value = i2c_gpio_sda_get();
	I2C_DELAY;
	i2c_gpio_scl_set(0);
	I2C_DELAY;
	I2C_DELAY;
	return value;
}

/* START: High -> Low on SDA while SCL is High */
static void i2c_gpio_send_start(void)
{
	I2C_DELAY;
	i2c_gpio_sda_set(1);
	I2C_DELAY;
	i2c_gpio_scl_set(1);
	I2C_DELAY;
	i2c_gpio_sda_set(0);
	I2C_DELAY;
}

/* STOP: Low -> High on SDA while SCL is High */
static void i2c_gpio_send_stop(void)
{
	i2c_gpio_scl_set(0);
	I2C_DELAY;
	i2c_gpio_sda_set(0);
	I2C_DELAY;
	i2c_gpio_scl_set(1);
	I2C_DELAY;
	i2c_gpio_sda_set(1);
	I2C_DELAY;
}

/* ack should be I2C_ACK or I2C_NOACK */
static void i2c_gpio_send_ack(int ack)
{
	i2c_gpio_write_bit(ack);
	i2c_gpio_scl_set(0);
	I2C_DELAY;
}

static void i2c_gpio_send_reset(void)
{
	int j;
	for (j = 0; j < 9; j++)
		i2c_gpio_write_bit(1);
	i2c_gpio_send_stop();
}

/* Set sda high with low clock, before reading slave data */
static void i2c_gpio_sda_high(void)
{
	i2c_gpio_scl_set(0);
	I2C_DELAY;
	i2c_gpio_sda_set(1);
	I2C_DELAY;
}

/* Send 8 bits and look for an acknowledgement */
static int i2c_gpio_write_byte(uchar data)
{
	int j;
	int nack;

	for (j = 0; j < 8; j++) {
		i2c_gpio_write_bit(data & 0x80);
		data <<= 1;
	}
	I2C_DELAY;
	/* Look for an <ACK>(negative logic) and return it */
	i2c_gpio_sda_high();
	nack = i2c_gpio_check_nack();
	return nack;	/* not a nack is an ack */
}

static int i2c_gpio_wwrite_byte(uchar data)
{
	int j;
	int nack;

	for (j = 0; j < 8; j++) {
		i2c_gpio_wwrite_bit(data & 0x80);
		data <<= 1;
	}
	I2C_DELAY;
	/* Look for an <ACK>(negative logic) and return it */
	i2c_gpio_sda_high();
	nack = i2c_gpio_check_nack();
	return nack;	/* not a nack is an ack */
}

static uchar i2c_gpio_read_byte(int ack)
{
	int  data;
	int  j;

	i2c_gpio_sda_high();
	data = 0;
	for (j = 0; j < 8; j++) {
		data <<= 1;
		data |= i2c_gpio_read_bit();
	}
	i2c_gpio_send_ack(ack);
	return data;
}

unsigned int i2c_gpio_write_eeprom(uint addr, int alen, uchar *buffer, int len)
{
	int shift, failures = 0;
	int index=0;
	char showt[32]={0};
	uchar wrbyte=0x0;

	i2c_gpio_send_start();
	if(i2c_gpio_write_byte(EEPROM_I2C_ADDRESS << 1)) {	/* write cycle */
		i2c_gpio_send_stop();
		puts("i2c_write, no chip responded\n");
		return(1);
	}
	shift = (alen-1) * 8;
	while(alen-- > 0) {
		if(i2c_gpio_write_byte(addr >> shift)) {
			puts("i2c_write, address not <ACK>ed\n");
			return(1);
		}
		shift -= 8;
	}

	for(index=0;index<len;index++)
	{
		wrbyte=buffer[index];
		if(i2c_gpio_wwrite_byte(wrbyte)) 
		{
			failures++;
		}
	}
	i2c_gpio_send_stop();
	udelay(100);
	i2c_gpio_send_reset();
	if(failures)
	{
		sprintf(showt," Error count:%d\n",failures);
		puts(showt);
	}
	udelay( 11000 );
	return(failures);
}

unsigned int i2c_gpio_read_eeprom(uint addr, int alen, uchar *buffer, int len)
{
	int shift;
	i2c_gpio_send_start();
	if(alen > 0) 
	{
		if(i2c_gpio_write_byte(EEPROM_I2C_ADDRESS << 1)) 
		{ /* write cycle */
			i2c_gpio_send_stop();
			puts("i2c_gpio_write_byte, Error !\n");
			return(1);
		}
		shift = (alen-1) * 8;
		while(alen-- > 0) 
		{
			if(i2c_gpio_write_byte(addr >> shift)) 
			{
				puts("i2c_read, address not <ACK>ed\n");
				return(1);
			}
			shift -= 8;
		}
	
		/* Some I2C chips need a stop/start sequence here,
		 * other chips don't work with a full stop and need
		 * only a start.  Default behaviour is to send the
		 * stop/start sequence.
		 */
#ifdef CONFIG_SOFT_I2C_READ_REPEATED_START
		i2c_gpio_send_start();
#else
		i2c_gpio_send_stop();
		i2c_gpio_send_start();
#endif
	}
	/*
	 * Send the chip address again, this time for a read cycle.
	 * Then read the data.	On the last byte, we do a NACK instead
	 * of an ACK(len == 0) to terminate the read.
	 */
	i2c_gpio_write_byte((EEPROM_I2C_ADDRESS << 1) | 1);	/* read cycle */
	while(len-- > 0) {
		*buffer++ = i2c_gpio_read_byte(len == 0);
	}
	i2c_gpio_send_stop();
	return(0);
}

static int i2c_gpio_eeprom_init(void)
{
	int ret;
	
	 i2c_gpio_send_start();
	 ret = i2c_gpio_write_byte((EEPROM_I2C_ADDRESS << 1) | 0);
	 i2c_gpio_send_stop();
	return ret;
}

#else
unsigned int eeprom_i2c_read( unsigned int addr, int alen, uint8_t *buffer, int len )
{
	i2c_set_bus_num( EEPROM_I2C_BUS );

	if ( i2c_read( EEPROM_I2C_ADDRESS, addr, alen, buffer, len ) )
	{
		puts( "I2C read failed in eeprom_i2c_read()\n" );
	}

	udelay( 10 );
	return(0);
}

unsigned int eeprom_i2c_write( unsigned int addr, int alen, uint8_t *buffer, int len )
{
	i2c_set_bus_num( EEPROM_I2C_BUS );

	if ( i2c_write( EEPROM_I2C_ADDRESS, addr, alen, buffer, len ) )
	{
		puts( "I2C write failed in eeprom_i2c_write()\n" );
	}
	udelay( 11000 );
	return(0);
}
#endif
int eeprom_i2c_parse_data(void)
{
	uchar buffer[128]={0};
	int i=0;
	int datalength=0;

	//if (AT24c02_eeprom.read( 0xfe, 1, buffer, 2)) {
#ifdef CONFIG_EEPROM_GPIO_I2C4
		if (i2c_gpio_read_eeprom( 0xfe, 1, buffer, 2)) {
#else
		if (eeprom_i2c_read( 0xfe, 1, buffer, 2)) {
#endif
		puts("I2C read failed in eeprom 0xf8()\n");
		AT24c02_eeprom.version=0x00;
		return 0;
	}

//	sprintf(buffer,"bAT24c02_eeprom.version(0x%04x)\n",AT24c02_eeprom.version);
	AT24c02_eeprom.version=0+buffer[0]+(buffer[1]<<8);
	if(AT24c02_eeprom.version<0x0 || AT24c02_eeprom.version>0xf000)
	{
		AT24c02_eeprom.version=0x00;
	}
	else
	{
		//AT24c02_eeprom.read( 0x00, 1, AT24c02_eeprom.content, 0x40);
#ifdef CONFIG_EEPROM_GPIO_I2C4
		i2c_gpio_read_eeprom( 0x00, 1, AT24c02_eeprom.content, 0x40);
#else
		eeprom_i2c_read( 0x00, 1, AT24c02_eeprom.content, 0x40);
#endif
	}
//	sprintf(buffer,"aAT24c02_eeprom.version(0x%04x)\n",AT24c02_eeprom.version);
//	puts(buffer);
	memset(&AT24c02_eeprom.data,0,sizeof(AT24c02_eeprom.data));
	AT24c02_eeprom.data.version=3;		
	i=0;
	datalength=0;
	while(i<0x40)
	{
		datalength=AT24c02_eeprom.content[i+1]+2;
		switch(AT24c02_eeprom.content[i])
		{
			default:
			case 0x00:
				return 0;
			case 0x01:
				memcpy(AT24c02_eeprom.data.mac1,AT24c02_eeprom.content+i,datalength);
				break;
			case 0x02:
				memcpy(AT24c02_eeprom.data.mac2,AT24c02_eeprom.content+i,datalength);
				break;
			case 0x03:
				memcpy(AT24c02_eeprom.data.softid,AT24c02_eeprom.content+i,datalength);
				break;
			case 0x04:
				memcpy(AT24c02_eeprom.data.backlight,AT24c02_eeprom.content+i,datalength);
				break;
			case 0x10:
				memcpy(AT24c02_eeprom.data.display,AT24c02_eeprom.content+i,datalength);
				break;
			case 0x11:
				memcpy(AT24c02_eeprom.data.logo,AT24c02_eeprom.content+i,datalength);
				break;
		}
		AT24c02_eeprom.size=i+datalength;
		i=i+datalength;
	}	
	return AT24c02_eeprom.version;
}

uchar datatransfer(uchar h4b,uchar l4b)
{
	uchar dtr=0x0;
	if(h4b>='0' && h4b<='9')
		dtr=(h4b-'0')<<4;
	else if(h4b>='a' && h4b<='f')
		dtr=(h4b-'a'+10)<<4;
	else if(h4b>='A' && h4b<='F')
		dtr=(h4b-'A'+10)<<4;
	
	if(l4b>='0' && l4b<='9')
		dtr+=(l4b-'0');
	else if(l4b>='a' && l4b<='f')
		dtr+=(l4b-'a'+10);
	else if(l4b>='A' && l4b<='F')
		dtr+=(l4b-'A'+10);
	return dtr;
}

int atoi(char *string)
{
	int length;
	int retval = 0;
	int i;
	int sign = 1;

	length = strlen(string);
	for (i = 0; i < length; i++) {
		if (0 == i && string[0] == '-') {
			sign = -1;
			continue;
		}
		if (string[i] > '9' || string[i] < '0') {
			break;
		}
		retval *= 10;
		retval += string[i] - '0';
	}
	retval *= sign;
	return retval;
}

u32 atoilength(char *string,int len)
{
	int length;
	u64 retval = 0;
	int i;
	u64 sign = 1;

	length = strlen(string);
	if(len<length)length=len;
	for (i = 0; i < length; i++) {
		if (0 == i && string[0] == '-') {
			sign = -1;
			continue;
		}
		if (string[i] > '9' || string[i] < '0') {
			break;
		}
		retval *= 10;
		retval += string[i] - '0';
	}
	retval *= sign;
	return retval;
}

int eeprom_i2c_synthesis_data(void)
{
	char tmp[128];
	uchar eepromtmp[10];

	puts("nMAC: ");
	sprintf(tmp,"write_systeminfo_eeprom:%02x:%02x:%02x:%02x:%02x:%02x\n",AT24c02_eeprom.data.mac1[2],AT24c02_eeprom.data.mac1[3],AT24c02_eeprom.data.mac1[4],AT24c02_eeprom.data.mac1[5],AT24c02_eeprom.data.mac1[6],AT24c02_eeprom.data.mac1[7]);
	puts(tmp);
	int write_size_offset=0;
	AT24c02_eeprom.size=0;
	memset(AT24c02_eeprom.content,0,sizeof(AT24c02_eeprom.content));
	if(AT24c02_eeprom.data.mac1[0]!=0 && AT24c02_eeprom.data.mac1[1]!=0)
	{
		memcpy(AT24c02_eeprom.content+AT24c02_eeprom.size,AT24c02_eeprom.data.mac1,AT24c02_eeprom.data.mac1[1]+2);
		AT24c02_eeprom.size+=(AT24c02_eeprom.data.mac1[1]+2);
	}
	if(AT24c02_eeprom.data.mac2[0]!=0 && AT24c02_eeprom.data.mac2[1]!=0)
	{
		memcpy(AT24c02_eeprom.content+AT24c02_eeprom.size,AT24c02_eeprom.data.mac2,AT24c02_eeprom.data.mac2[1]+2);
		AT24c02_eeprom.size+=(AT24c02_eeprom.data.mac2[1]+2);
	}
	if(AT24c02_eeprom.data.softid[0]!=0 && AT24c02_eeprom.data.softid[1]!=0)
	{
		memcpy(AT24c02_eeprom.content+AT24c02_eeprom.size,AT24c02_eeprom.data.softid,AT24c02_eeprom.data.softid[1]+2);
		AT24c02_eeprom.size+=(AT24c02_eeprom.data.softid[1]+2);
	}
	if(AT24c02_eeprom.data.backlight[0]!=0 && AT24c02_eeprom.data.backlight[1]!=0)
	{
		memcpy(AT24c02_eeprom.content+AT24c02_eeprom.size,AT24c02_eeprom.data.backlight,AT24c02_eeprom.data.backlight[1]+2);
		AT24c02_eeprom.size+=(AT24c02_eeprom.data.backlight[1]+2);
	}
	if(AT24c02_eeprom.data.display[0]!=0 && AT24c02_eeprom.data.display[1]!=0)
	{
		memcpy(AT24c02_eeprom.content+AT24c02_eeprom.size,AT24c02_eeprom.data.display,AT24c02_eeprom.data.display[1]+2);
		AT24c02_eeprom.size+=(AT24c02_eeprom.data.display[1]+2);
	}
	if(AT24c02_eeprom.data.logo[0]!=0 && AT24c02_eeprom.data.logo[1]!=0)
	{
		memcpy(AT24c02_eeprom.content+AT24c02_eeprom.size,AT24c02_eeprom.data.logo,AT24c02_eeprom.data.logo[1]+2);
		AT24c02_eeprom.size+=(AT24c02_eeprom.data.logo[1]+2);
	}

	while(write_size_offset<AT24c02_eeprom.size)
	{
		//AT24c02_eeprom.write(write_size_offset, 1, AT24c02_eeprom.content+write_size_offset, 8);
#ifdef CONFIG_EEPROM_GPIO_I2C4
		i2c_gpio_write_eeprom(write_size_offset, 1, AT24c02_eeprom.content+write_size_offset, 8);
#else
		eeprom_i2c_write(write_size_offset, 1, AT24c02_eeprom.content+write_size_offset, 8);
#endif
		write_size_offset+=8;
	}
	AT24c02_eeprom.version=AT24c02_eeprom.data.version;
	eepromtmp[0]=0xff & AT24c02_eeprom.data.version;
	eepromtmp[1]=0xff & (AT24c02_eeprom.data.version>>8);
	//AT24c02_eeprom.write(0xfe, 1, eepromtmp, 2);
#ifdef CONFIG_EEPROM_GPIO_I2C4
	i2c_gpio_write_eeprom(0xfe, 1, eepromtmp, 2);
#else
	eeprom_i2c_write(0xfe, 1, eepromtmp, 2);
#endif
	sprintf(tmp,"AT24c02_eeprom.size:%d\n",AT24c02_eeprom.size);
	puts(tmp);

	return AT24c02_eeprom.version;
}

int Load_config_from_mmc(void)
{
	struct mmc *mmc;

	mmc = find_mmc_device(1);
	if (mmc) 
	{
		mmc_init(mmc);
	}
	if (mmc) 
	{
		long size;
		//int i;
		char tmp[128];
		uchar tbuffer[512]={0};
		//uchar buffer[128]={0};
		char * ptr;
		block_dev_desc_t *dev_desc=NULL;
		unsigned int read_version=0;
		//unsigned int tmpversion=AT24c02_eeprom.data.version;
		unsigned int prm_check=0;
				
		dev_desc=get_dev("mmc",1);
		if (dev_desc!=NULL) 
		{
			if (fat_register_device((block_dev_desc_t *)dev_desc,1)==0) 
			{
				size = file_fat_read ("aplex.cfg", (void *) tbuffer, sizeof(tbuffer));
				if(size<=0)return -1;
				puts((char *)tbuffer);
				puts("\n");
				ptr = strstr((const char *)tbuffer, "Version=");
				if (ptr != NULL) 
				{
					read_version=atoi(ptr+8);
					if(read_version<0 || read_version>0xF000)
						read_version=AT24c02_eeprom.version;
				}
				else
				{
					read_version=AT24c02_eeprom.version;
				}
				sprintf(tmp,"Version=0x%04x\n",read_version);
				puts(tmp);
				if(read_version!=0x03)
				{
					return -2;
				}
				ptr = strstr((const char *)tbuffer, "MAC1=");
				if (ptr != NULL)
				{
					//70:B3:D5:10:6F:56 
					//70:B3:D5:10:6F:57
					//00:11:22:33:44:55
					AT24c02_eeprom.show_pass_logo++;
					ptr+=5;
					memset(tmp,0x0,sizeof(tmp));
					tmp[0]=datatransfer(ptr[0], ptr[1]);
					tmp[1]=datatransfer(ptr[3], ptr[4]);
					tmp[2]=datatransfer(ptr[6], ptr[7]);
					tmp[3]=datatransfer(ptr[9], ptr[10]);
					tmp[4]=datatransfer(ptr[12],ptr[13]);
					tmp[5]=datatransfer(ptr[15],ptr[16]);
					if (is_valid_ethaddr((const u8 *)tmp))
					{
						AT24c02_eeprom.data.mac1[0]=0x01;
						AT24c02_eeprom.data.mac1[1]=0x06;
						memcpy(AT24c02_eeprom.data.mac1+2,tmp,6);
					}
					//sprintf(tmp,"%02x:%02x:%02x:%02x:%02x:%02x",AT24c02_eeprom.data.mac1[2],AT24c02_eeprom.data.mac1[3],AT24c02_eeprom.data.mac1[4],AT24c02_eeprom.data.mac1[5],AT24c02_eeprom.data.mac1[6],AT24c02_eeprom.data.mac1[7]);
					//puts("nMAC: ");
					//puts(tmp);
					//puts("\n");
				}
				ptr = strstr((const char *)tbuffer, "Software_part_number=");
				if (ptr != NULL) 
				{
					u32 softidhigh=0;
					u32 softidlow=0;
					AT24c02_eeprom.show_pass_logo++;
					softidhigh=(u32)(atoilength(ptr+21,4));
					softidlow=(u32)(atoilength(ptr+21+4,8));
					sprintf(tmp,"Software_part_number=(%04u)(%08u)\n",softidhigh,softidlow);
					puts(tmp);
					if(softidhigh<=9999 && softidhigh>0 && softidlow<=99999999 && softidlow>0)
					{
						AT24c02_eeprom.data.softid[0]=0x03;
						AT24c02_eeprom.data.softid[1]=0x06;
						AT24c02_eeprom.data.softid[2]=(softidhigh>>8)&0xff;
						AT24c02_eeprom.data.softid[3]=(softidhigh>>0)&0xff;
						AT24c02_eeprom.data.softid[4]=(softidlow>>24)&0xff;
						AT24c02_eeprom.data.softid[5]=(softidlow>>16)&0xff;
						AT24c02_eeprom.data.softid[6]=(softidlow>> 8)&0xff;
						AT24c02_eeprom.data.softid[7]=(softidlow>> 0)&0xff;
					}
				}

				//for backlight pwm parameter
				prm_check=0;
				ptr = strstr((const char *)tbuffer, "Backlight_polarity=");
				if (ptr != NULL) 
				{
					prm_check++;
					AT24c02_eeprom.data.backlight[2]=(uchar)(atoi(ptr+19)&0xff)==1?1:0;
				}
				ptr = strstr((const char *)tbuffer, "Backlight_min=");
				if (ptr != NULL) 
				{
					prm_check++;
					AT24c02_eeprom.data.backlight[3]=(uchar)(atoi(ptr+14)&0xff);
				}
				ptr = strstr((const char *)tbuffer, "Backlight_frequency=");
				if (ptr != NULL) 
				{
					int read_backlight_frequency=atoi(ptr+20);
					prm_check++;
					AT24c02_eeprom.data.backlight[4]=(read_backlight_frequency>>8)&0xff;
					AT24c02_eeprom.data.backlight[5]=(read_backlight_frequency)&0xff;
				}
				if(prm_check)
				{
					int check_backlight_frequency=AT24c02_eeprom.data.backlight[4]*256+AT24c02_eeprom.data.backlight[5];
					AT24c02_eeprom.show_pass_logo++;
					AT24c02_eeprom.data.backlight[0]=0x04;
					AT24c02_eeprom.data.backlight[1]=0x04;
					if(AT24c02_eeprom.data.backlight[2]!=1 &&AT24c02_eeprom.data.backlight[2]!=0)
						AT24c02_eeprom.data.backlight[2]=0;
					if(AT24c02_eeprom.data.backlight[3]<0 || AT24c02_eeprom.data.backlight[3]>40)//default :0
						AT24c02_eeprom.data.backlight[3]=0;
					if(check_backlight_frequency<100 || check_backlight_frequency>50000)
					{
						check_backlight_frequency=50000;
						AT24c02_eeprom.data.backlight[4]=(check_backlight_frequency>>8)&0xff;
						AT24c02_eeprom.data.backlight[5]=(check_backlight_frequency)&0xff;
					}
				}

				//for display parameter
				prm_check=0;
				ptr = strstr((const char *)tbuffer, "Resolution_ID=");
				if (ptr != NULL) 
				{
					prm_check++;
					AT24c02_eeprom.data.display[2]=(uchar)(atoi(ptr+14)&0xff);
				}
				ptr = strstr((const char *)tbuffer, "Color_depth=");
				if (ptr != NULL) 
				{
					prm_check++;
					AT24c02_eeprom.data.display[3]=(uchar)(atoi(ptr+12)&0xff);
				}
				ptr = strstr((const char *)tbuffer, "Frame_rate=");
				if (ptr != NULL) 
				{
					prm_check++;
					AT24c02_eeprom.data.display[4]=(uchar)(atoi(ptr+11)&0xff);
				}
				ptr = strstr((const char *)tbuffer, "Interface=");
				if (ptr != NULL) 
				{
					prm_check++;
					AT24c02_eeprom.data.display[5]=(uchar)(atoi(ptr+10)&0xff);
				}
				if(prm_check)
				{
					AT24c02_eeprom.show_pass_logo++;
					AT24c02_eeprom.data.display[0]=0x10;
					AT24c02_eeprom.data.display[1]=0x04;
					if(AT24c02_eeprom.data.display[2]<RESOLUTION_640X480 || AT24c02_eeprom.data.display[2]>RESOLUTION_1920X1080)
						AT24c02_eeprom.data.display[2]=RESOLUTION_800X480;
					if(AT24c02_eeprom.data.display[3]!=0x12 && AT24c02_eeprom.data.display[3]!=0x18)
						AT24c02_eeprom.data.display[3]=0x12;
					if(AT24c02_eeprom.data.display[4]<0x28 || AT24c02_eeprom.data.display[4]>0x64)
						AT24c02_eeprom.data.display[4]=0x3c;
					//
					if(AT24c02_eeprom.data.display[5]<1 || AT24c02_eeprom.data.display[5]>3)
						AT24c02_eeprom.data.display[5]=1;
				}
				ptr = strstr((const char *)tbuffer, "Logo=");
				if (ptr != NULL) 
				{
					uchar read_boot_logo=AT24c02_eeprom.data.logo[2];
					AT24c02_eeprom.show_pass_logo++;
					read_boot_logo=(uchar)(atoi(ptr+5)&0xff);
					if(read_boot_logo<1 || read_boot_logo>5)
					{
						read_boot_logo=1;
					}
					AT24c02_eeprom.data.logo[0]=0x11;
					AT24c02_eeprom.data.logo[1]=0x01;
					AT24c02_eeprom.data.logo[2]=0xff&read_boot_logo;
				}
				ptr = strstr((const char *)tbuffer, "Halt");
				if (ptr != NULL) 
				{
					AT24c02_eeprom.mHalt=1;
				}
				ptr = strstr((const char *)tbuffer, "bootdelay=");
				if (ptr != NULL) 
				{
					AT24c02_eeprom.mbootdelay=(uchar)(atoi(ptr+10)&0xff);
				}
				if(AT24c02_eeprom.show_pass_logo)
					eeprom_i2c_synthesis_data();
			}
		}
	}
	return 0;
}

unsigned char eeprom_i2c_get_type(void)
{
	return AT24c02_eeprom.data.display[5];
}

unsigned char eeprom_i2c_get_color_depth(void)
{
	if(AT24c02_eeprom.data.display[3]!=0x18)
		AT24c02_eeprom.data.display[3]=0x12;
	return AT24c02_eeprom.data.display[3];
}

unsigned char eeprom_i2c_pass_logo(void)
{
	return AT24c02_eeprom.show_pass_logo;
}

unsigned char eeprom_i2c_get_EDID(void)
{
	return AT24c02_eeprom.data.display[2];
}
//#define CMDLINE_ADD_QUIET
#define CONSOLE_READWRITE_ABLE
void set_kernel_env(int width, int height)
{
	char videoprm[128]={0};
	char Backlightprm[128]={0};
#ifdef MX6_SABRE_ANDROID_COMMON_H
	char consoleprm[128]={0};
#endif
	char envprm[512]={0};

	if(AT24c02_eeprom.data.display[4]<30 || AT24c02_eeprom.data.display[4]>100)
		AT24c02_eeprom.data.display[4]=60;
	if(AT24c02_eeprom.data.display[3]!=18 && AT24c02_eeprom.data.display[3]!=24)
		AT24c02_eeprom.data.display[3]=18;
//
	switch(AT24c02_eeprom.data.display[5])
	{//0x01:lvds, 0x02:hdmi, 0x03:RGB
		case 0x02:
//			sprintf(videoprm,"video=mxcfb0:dev=hdmi,%dx%dM@%u,if=RGB%d,bpp=32 video=mxcfb1:off video=mxcfb2:off vmalloc=256M", width, height, AT24c02_eeprom.data.display[4], AT24c02_eeprom.data.display[3]==18?666:24);
			sprintf(videoprm,"video=mxcfb0:dev=hdmi,%dx%dM@%u,if=RGB24,bpp=32 video=mxcfb1:off video=mxcfb2:off vmalloc=%dM", width, height, AT24c02_eeprom.data.display[4],192+24*AT24c02_eeprom.data.display[2]);
			break;
		case 0x01:
		default:
			AT24c02_eeprom.data.display[5]=0x01;
			if(eeprom_i2c_get_EDID()==RESOLUTION_1920X1080)
				sprintf(videoprm,"video=mxcfb0:dev=ldb,%dx%dM@%u,if=RGB%d,bpp=32 ldb=spl1 video=mxcfb1:off video=mxcfb2:off vmalloc=384M", width, height, AT24c02_eeprom.data.display[4], AT24c02_eeprom.data.display[3]==18?666:24);
			else
				sprintf(videoprm,"video=mxcfb0:dev=ldb,%dx%dM@%u,if=RGB%d,bpp=32 video=mxcfb1:off video=mxcfb2:off vmalloc=256M", width, height, AT24c02_eeprom.data.display[4], AT24c02_eeprom.data.display[3]==18?666:24);
			break;
	}
#ifdef MX6_SABRE_ANDROID_COMMON_H
#ifdef CONSOLE_READWRITE_ABLE
	#ifdef CMDLINE_ADD_QUIET
	sprintf(consoleprm,"androidboot.console=ttymxc0 androidboot.selinux=disabled consoleblank=0 quiet");
	#else
	sprintf(consoleprm,"androidboot.console=ttymxc0 androidboot.selinux=disabled consoleblank=0");
	#endif
#else
	#ifdef CMDLINE_ADD_QUIET
	sprintf(consoleprm,"androidboot.console=ttymxc0 consoleblank=0 quiet");
	#else
	sprintf(consoleprm,"androidboot.console=ttymxc0 consoleblank=0");
	#endif
#endif
	if(AT24c02_eeprom.data.backlight[0]==0x04 && AT24c02_eeprom.data.backlight[1]==0x04)
	{
		int check_backlight_frequency=AT24c02_eeprom.data.backlight[4]*256+AT24c02_eeprom.data.backlight[5];
		sprintf(Backlightprm,"Backlight_polarity=%d,Backlight_min=%d,Backlight_frequency=%d",AT24c02_eeprom.data.backlight[2]?1:0,AT24c02_eeprom.data.backlight[3],check_backlight_frequency);
		sprintf(envprm,"console=ttymxc0,115200 init=/init %s %s %s androidboot.hardware=freescale cma=384M usbcore.autosuspend=-1",videoprm,consoleprm,Backlightprm);
	}
	else
	{
		sprintf(envprm,"console=ttymxc0,115200 init=/init %s %s androidboot.hardware=freescale cma=384M usbcore.autosuspend=-1",videoprm,consoleprm);
	}
	setenv("bootargs",envprm);
#else
	//mmcargs=setenv bootargs console=${console},${baudrate} ${smp}  root=${mmcroot}\0"	
	if(AT24c02_eeprom.data.backlight[0]==0x04 && AT24c02_eeprom.data.backlight[1]==0x04)
	{
		int check_backlight_frequency=AT24c02_eeprom.data.backlight[4]*256+AT24c02_eeprom.data.backlight[5];
		sprintf(Backlightprm,"Backlight_polarity=%d,Backlight_min=%d,Backlight_frequency=%d",AT24c02_eeprom.data.backlight[2]?1:0,AT24c02_eeprom.data.backlight[3],check_backlight_frequency);
#ifdef CMDLINE_ADD_QUIET
		sprintf(envprm,"setenv bootargs console=${console},${baudrate} ${smp} root=${mmcroot} %s %s usbcore.autosuspend=-1 quiet",videoprm,Backlightprm);
#else
		sprintf(envprm,"setenv bootargs console=${console},${baudrate} ${smp} root=${mmcroot} %s %s usbcore.autosuspend=-1",videoprm,Backlightprm);
#endif
	}
	else
	{
#ifdef CMDLINE_ADD_QUIET
		sprintf(envprm,"setenv bootargs console=${console},${baudrate} ${smp} root=${mmcroot} %s usbcore.autosuspend=-1 quiet",videoprm);
#else
		sprintf(envprm,"setenv bootargs console=${console},${baudrate} ${smp} root=${mmcroot} %s usbcore.autosuspend=-1",videoprm);
#endif
	}
	setenv("mmcargs",envprm);
#endif
	if(is_valid_ethaddr(AT24c02_eeprom.data.mac1+2))
	{
		sprintf(envprm,"%02x:%02x:%02x:%02x:%02x:%02x", AT24c02_eeprom.data.mac1[2], AT24c02_eeprom.data.mac1[3], AT24c02_eeprom.data.mac1[4], AT24c02_eeprom.data.mac1[5], AT24c02_eeprom.data.mac1[6], AT24c02_eeprom.data.mac1[7]);
		setenv("ethaddr",envprm);
		setenv("fec_addr",envprm);
	}
	if(AT24c02_eeprom.mHalt>0)
		setenv("Halt","y");
	if(AT24c02_eeprom.mbootdelay>0)
	{
		sprintf(envprm,"%d", AT24c02_eeprom.mbootdelay&0x0f);
		setenv("bootdelay",envprm);
	}
}

int eeprom_i2c_init(void)
{
	memset(&AT24c02_eeprom,0,sizeof(AT24c02_eeprom));
 	AT24c02_eeprom.accessable_size = EEPROM_ACCESSABLE_SIZE;
	AT24c02_eeprom.max_size = EEPROM_MAX_SIZE;
	AT24c02_eeprom.i2c_bus = EEPROM_I2C_BUS;
	AT24c02_eeprom.device_addr = EEPROM_I2C_ADDRESS;
	AT24c02_eeprom.address_length = EEPROM_ADDRESS_LENGTH;
	AT24c02_eeprom.mmc_bus = MMC_DEVICE_BUS;

//#ifdef CONFIG_EEPROM_GPIO_I2C4
//	AT24c02_eeprom.read = i2c_gpio_read_eeprom;
//	AT24c02_eeprom.write = i2c_gpio_write_eeprom;
//#else
//	AT24c02_eeprom.read = eeprom_i2c_read;
//	AT24c02_eeprom.write = eeprom_i2c_write;
//#endif
	AT24c02_eeprom.parse_data = eeprom_i2c_parse_data;
	AT24c02_eeprom.synthesis_data = eeprom_i2c_synthesis_data;
	AT24c02_eeprom.load_config = Load_config_from_mmc;

	AT24c02_eeprom.show_pass_logo=0;
	AT24c02_eeprom.mHalt=0;
	AT24c02_eeprom.mbootdelay=0;

#ifdef CONFIG_EEPROM_GPIO_I2C4
	i2c_gpio_eeprom_init();
#endif
	eeprom_i2c_parse_data();
	return 0;
}

