/*
 * Microchip SPI Touchscreen Driver
 *
 * Copyright (c) 2011 Microchip Technology, Inc.
 * 
 * http://www.microchip.com/mtouch
 */

/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

#include <linux/input.h>	
#include <linux/spi/spi.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <asm/setup.h>

/* The maximum packet byte length */
#define MCHIP_MAX_LENGTH 5
#define FIVE_POINT_CALIBRATION
/* The private data structure that is referenced within the SPI bus driver */
struct ar1021_spi_priv {
	struct spi_device *client;
	struct input_dev *input;
	struct work_struct work;
	int irq;
	int testCount;
};

/* These are all the sysfs variables used to store and retrieve information
   from a user-level application */
static char sendBuffer[100];
static char receiveBuffer[100];
static int commandMode=0;
static int eeprom_read_mode_flag=0;
static int eeprom_write_mode[7]={0};
static int commandDataPending=0;
#ifdef FIVE_POINT_CALIBRATION
static int Xorg=0;
static int Yorg=0;
static int XLorg=0;
static int YLorg=0;
static int XFactory=0;
static int YFactory=0;
static int eeprom_Xorg=0;
static int eeprom_Yorg=0;
static int eeprom_XLorg=0;
static int eeprom_YLorg=0;
static int eeprom_XFactory=0;
static int eeprom_YFactory=0;
#else
static int minX=0;
static int maxX=4095;
static int minY=0;
static int maxY=4095;
static int invertX=0;
static int invertY=0;
static int eeprom_minX=0;
static int eeprom_maxX=4095;
static int eeprom_minY=0;
static int eeprom_maxY=4095;
static int eeprom_invertX=0;
static int eeprom_invertY=0;
static int eeprom_swapAxes=0;
#endif
static int swapAxes=0;
static int lastPUCoordX=0;
static int lastPUCoordY=0;

/* These variables allows the IRQ to be specified via a module parameter
   or kernel parameter.  To configuration of these value, please see 
   driver documentation. */
static int touchIRQ=-1;
static int probeForIRQ=0;
static int testSPIdata=0;
static int probeMin=0;
static int probeMax=200;

module_param(touchIRQ, int, S_IRUGO);
module_param(probeMin, int, S_IRUGO);
module_param(probeMax, int, S_IRUGO);
module_param(probeForIRQ, int, S_IRUGO);
module_param(testSPIdata, int, S_IRUGO);

/* Since the reference to private data is stored within the SPI
   bus driver, we will store another reference within this driver
   so the sysfs related function may also access this data */
struct ar1021_spi_priv *privRef=NULL;

/**********************************************************************
Function:
	commandDataPending_show()

Description:
	Display value of "commandDataPending" variable to application that is
	requesting it's value.	
**********************************************************************/
static ssize_t commandDataPending_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	return sprintf(buf, "%d", commandDataPending);
}

/******************************************************************************
Function:
	commandDataPending_store()

Description:
	Save value to "commandDataPending" variable from application that is
	requesting this.	
******************************************************************************/
static ssize_t commandDataPending_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	sscanf(buf, "%d", &commandDataPending);
	return count;
}

static struct kobj_attribute commandDataPending_attribute =
	__ATTR(commandDataPending, 0660, commandDataPending_show, commandDataPending_store);

/******************************************************************************
Function:
	commandMode_show()

Description:
	Display value of "commandMode" variable to application that is
	requesting it's value.	
******************************************************************************/
static ssize_t commandMode_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	return sprintf(buf, "%d", commandMode);
}

/******************************************************************************
Function:
	commandMode_store()

Description:
	Save value to "commandMode" variable from application that is
	requesting this.	
******************************************************************************/
static ssize_t commandMode_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	sscanf(buf, "%d", &commandMode);
	return count;

}

static struct kobj_attribute commandMode_attribute =
	__ATTR(commandMode, 0660, commandMode_show, commandMode_store);

/******************************************************************************
Function:
	receiveBuffer_show()

Description:
	Display value of "receiveBuffer" variable to application that is
	requesting it's value.	
******************************************************************************/
static ssize_t receiveBuffer_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	/* since we have now read the receiveBuffer, receive data is no longer pending */
	commandDataPending=0;
	return sprintf(buf, "%s", receiveBuffer);
}

/******************************************************************************
Function:
	receiveBuffer_store()

Description:
	Save value to "receiveBuffer" variable from application that is
	requesting this.	
******************************************************************************/
static ssize_t receiveBuffer_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	return snprintf(receiveBuffer,sizeof(receiveBuffer),"%s",buf);
}

static struct kobj_attribute receiveBuffer_attribute =
	__ATTR(receiveBuffer, 0660, receiveBuffer_show, receiveBuffer_store);

/******************************************************************************
Function:
	sendBuffer_show()

Description:
	Display value of "sendBuffer" variable to application that is
	requesting it's value.	
******************************************************************************/
static ssize_t sendBuffer_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	return sprintf(buf, "%s", sendBuffer);
}

/******************************************************************************
Function:
	sendBuffer_store()

Description:
	Save value to "sendBuffer" variable from application that is
	requesting this.	
******************************************************************************/
static ssize_t sendBuffer_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	int commandByte[8];
	int numCommandBytes;
	int i;

	commandDataPending=0;

	/* disallow commands to be sent until command mode is enabled */
	if (0==commandMode)
	{
		printk("ar1021 SPI: Warning: command bytes will be ignored until commandMode is enabled\n");
		strcpy(sendBuffer,"");
		return count;
	}

	numCommandBytes=sscanf(buf,"%x %x %x %x %x %x %x %x",&commandByte[0],&commandByte[1],&commandByte[2],&commandByte[3],&commandByte[4],&commandByte[5],&commandByte[6],&commandByte[7]);

	printk(KERN_DEBUG "ar1021 SPI: Processed %d bytes.\n",numCommandBytes); 

	/* Verify command string to send to controller is valid */
	if (numCommandBytes<3) 
	{
		printk("ar1021 SPI: Insufficient command bytes to process.\n");
	}
	else if (commandByte[0]!=0x55)
	{
		printk("ar1021 SPI: Invalid header byte (0x55 expected).\n");
	}	
	else if (commandByte[1] != (numCommandBytes-2))
	{
		printk("ar1021 SPI: Number of command bytes specified not valid for current string.\n");
	}	

	strcpy(sendBuffer,"");
	printk(KERN_DEBUG "ar1021 SPI: sending command bytes: "); 
	for (i=0;i<numCommandBytes;i++)
	{
		printk(KERN_DEBUG "0x%02x ",commandByte[i]);
		spi_write(privRef->client,(unsigned char *)&commandByte[i],1);
	}
	printk(KERN_DEBUG "\n");

	return snprintf(sendBuffer,sizeof(sendBuffer),"%s",buf);
}

static struct kobj_attribute sendBuffer_attribute =
	__ATTR(sendBuffer, 0660, sendBuffer_show, sendBuffer_store);

/******************************************************************************
Function:
	calibrationSettings_show()

Description:
	Display value of "calibrationSettings" variable to application that is
	requesting it's value.	The handling of the calibration settings has
	been grouped together.
******************************************************************************/
static ssize_t calibrationSettings_show(struct kobject *kobj, struct kobj_attribute *attr,
		      char *buf)
{
	int calibrationSetting=0;

	if (strcmp(attr->attr.name, "lastPUCoordX") == 0)
		calibrationSetting = lastPUCoordX;
	else if (strcmp(attr->attr.name, "lastPUCoordY") == 0)
		calibrationSetting = lastPUCoordY;
	else if (strcmp(attr->attr.name, "swapAxes") == 0)
		calibrationSetting = swapAxes;
#ifdef FIVE_POINT_CALIBRATION
	else if (strcmp(attr->attr.name, "Xorg") == 0)
		calibrationSetting = Xorg;
	else if (strcmp(attr->attr.name, "Yorg") == 0)
		calibrationSetting = Yorg;
	else if (strcmp(attr->attr.name, "XLorg") == 0)
		calibrationSetting = XLorg;
	else if (strcmp(attr->attr.name, "YLorg") == 0)
		calibrationSetting = YLorg;
	else if (strcmp(attr->attr.name, "XFactory") == 0)
		calibrationSetting = XFactory;
	else if (strcmp(attr->attr.name, "YFactory") == 0)
		calibrationSetting = YFactory;
#else
	else if (strcmp(attr->attr.name, "minX") == 0)
		calibrationSetting = minX;
	else if (strcmp(attr->attr.name, "maxX") == 0)
		calibrationSetting = maxX;
	else if (strcmp(attr->attr.name, "minY") == 0)
		calibrationSetting = minY;
	else if (strcmp(attr->attr.name, "maxY") == 0)
		calibrationSetting = maxY;
	else if (strcmp(attr->attr.name, "invertX") == 0)
		calibrationSetting = invertX;
	else if (strcmp(attr->attr.name, "invertY") == 0)
		calibrationSetting = invertY;
#endif

	return sprintf(buf, "%d\n", calibrationSetting);
}

static int ar1021_suspend(struct device *dev)
{
	printk("ar1021_suspend\n");	
	return 0;
}

static int ar1021_resume(struct device *dev)
{
	printk("ar1021_resume\n");	
	return 0;
}
/******************************************************************************
Function:
	calibrationSettings_store()

Description:
	Save calibration setting to corresponding variable from application 
	that is requesting this.	
******************************************************************************/
static ssize_t calibrationSettings_store(struct kobject *kobj, struct kobj_attribute *attr,
		       const char *buf, size_t count)
{
	int calibrationSetting;
	int i=0;

	sscanf(buf, "%d", &calibrationSetting);

	if (strcmp(attr->attr.name, "lastPUCoordX") == 0)
		lastPUCoordX = calibrationSetting;
	else if (strcmp(attr->attr.name, "lastPUCoordY") == 0)
		lastPUCoordY = calibrationSetting;
	else if (strcmp(attr->attr.name, "swapAxes") == 0)
		swapAxes = calibrationSetting;
#ifdef FIVE_POINT_CALIBRATION
	else if (strcmp(attr->attr.name, "XFactory") == 0)
	{
		XFactory = calibrationSetting;
		if(calibrationSetting>=-20000 && calibrationSetting<=20000 && calibrationSetting!=0)
		{
			if(eeprom_XFactory!=XFactory)
			{
				unsigned char buff[16]={0};
				int minusflag=XFactory>=0?0:1;
				int tmpvalue=XFactory>=0?XFactory:-1*XFactory;
				
				eeprom_write_mode[0]=1;
				buff[0]=0x55;
				buff[1]=0x06;
				buff[2]=0x29;
				buff[3]=0x00;
				buff[4]=0x90;
				buff[5]=0x02;
				buff[6]=0xff&tmpvalue;
				buff[7]=0xff&(tmpvalue>>8);
				if(minusflag)
					buff[7]|=0x80;
				for (i=0;i<8;i++)
				{
					spi_write(privRef->client,(unsigned char *)&buff[i],1);
				}
				msleep(5);
				eeprom_XFactory=XFactory;
				eeprom_write_mode[0]=0;
			}
		}
	}
	else if (strcmp(attr->attr.name, "YFactory") == 0)
	{
		YFactory = calibrationSetting;
		if(calibrationSetting>=-20000 && calibrationSetting<=20000 && calibrationSetting!=0)
		{
			if(eeprom_YFactory!=YFactory)
			{
				unsigned char buff[16]={0};
				int minusflag=YFactory>=0?0:1;
				int tmpvalue=YFactory>=0?YFactory:-1*YFactory;
				
				eeprom_write_mode[1]=1;
				buff[0]=0x55;
				buff[1]=0x06;
				buff[2]=0x29;
				buff[3]=0x00;
				buff[4]=0x92;
				buff[5]=0x02;
				buff[6]=0xff&tmpvalue;
				buff[7]=0xff&(tmpvalue>>8);
				if(minusflag)
					buff[7]|=0x80;
				for (i=0;i<8;i++)
				{
					spi_write(privRef->client,(unsigned char *)&buff[i],1);
				}
				msleep(5);
				eeprom_YFactory=YFactory;
				eeprom_write_mode[1]=0;
			}
		}
	}
	else if (strcmp(attr->attr.name, "Xorg") == 0)
	{
		Xorg = calibrationSetting;
		if(calibrationSetting>=0 && calibrationSetting<=4095 && XFactory!=0 && YFactory!=0)
		{
			if(eeprom_Xorg!=Xorg)
			{
				unsigned char buff[16]={0}; 	
				eeprom_write_mode[4]=1;
				buff[0]=0x55;
				buff[1]=0x06;
				buff[2]=0x29;
				buff[3]=0x00;
				buff[4]=0x98;
				buff[5]=0x02;
				buff[6]=0xff&Xorg;
				buff[7]=0xff&(Xorg>>8);
				if(swapAxes)
					buff[7]|=0x10;
				for (i=0;i<8;i++)
				{
					spi_write(privRef->client,(unsigned char *)&buff[i],1);
				}
				msleep(5);
				eeprom_Xorg=Xorg;
				eeprom_write_mode[4]=0;
			}
		}
	}	
	else if (strcmp(attr->attr.name, "Yorg") == 0)
	{
		Yorg = calibrationSetting;
		if(calibrationSetting>=0 && calibrationSetting<=4095 && XFactory!=0 && YFactory!=0)
		{
			if(eeprom_Yorg!=Yorg)
			{
				unsigned char buff[16]={0}; 	
				eeprom_write_mode[5]=1;
				buff[0]=0x55;
				buff[1]=0x06;
				buff[2]=0x29;
				buff[3]=0x00;
				buff[4]=0x9A;
				buff[5]=0x02;
				buff[6]=0xff&Yorg;
				buff[7]=0xff&(Yorg>>8);
				if(swapAxes)
					buff[7]|=0x10;
				for (i=0;i<8;i++)
				{
					spi_write(privRef->client,(unsigned char *)&buff[i],1);
				}
				msleep(5);
				eeprom_Yorg=Yorg;
				eeprom_write_mode[5]=0;
			}
		}
	}	
	else if (strcmp(attr->attr.name, "XLorg") == 0)
	{
		XLorg = calibrationSetting;
		if(calibrationSetting>=0 && calibrationSetting<=4095 && XFactory!=0 && YFactory!=0)
		{
			if(eeprom_XLorg!=XLorg)
			{
				unsigned char buff[16]={0}; 	
				eeprom_write_mode[2]=1;
				buff[0]=0x55;
				buff[1]=0x06;
				buff[2]=0x29;
				buff[3]=0x00;
				buff[4]=0x94;
				buff[5]=0x02;
				buff[6]=0xff&XLorg;
				buff[7]=0xff&(XLorg>>8);
				if(swapAxes)
					buff[7]|=0x10;
				for (i=0;i<8;i++)
				{
					spi_write(privRef->client,(unsigned char *)&buff[i],1);
				}
				msleep(5);
				eeprom_XLorg=XLorg;
				eeprom_write_mode[2]=0;
			}
		}
	}
	else if (strcmp(attr->attr.name, "YLorg") == 0)
	{
		YLorg = calibrationSetting;
		if(calibrationSetting>=0 && calibrationSetting<=4095 && XFactory!=0 && YFactory!=0)
		{
			if(eeprom_YLorg!=YLorg)
			{
				unsigned char buff[16]={0}; 	
				
				eeprom_write_mode[3]=1;
				buff[0]=0x55;
				buff[1]=0x06;
				buff[2]=0x29;
				buff[3]=0x00;
				buff[4]=0x96;
				buff[5]=0x02;
				buff[6]=0xff&YLorg;
				buff[7]=0xff&(YLorg>>8);
				if(swapAxes)
					buff[7]|=0x10;
				for (i=0;i<8;i++)
				{
					spi_write(privRef->client,(unsigned char *)&buff[i],1);
				}
				//msleep(5);
				eeprom_YLorg=YLorg;
				eeprom_write_mode[3]=0;
			}
		}
	}
#else
	else if (strcmp(attr->attr.name, "minX") == 0)
	{
		minX = calibrationSetting;
		if(calibrationSetting>0 && calibrationSetting<4095)
			if(eeprom_minX!=minX)
			{
				unsigned char buff[16]={0}; 	
				
				eeprom_write_mode[0]=1;
				buff[0]=0x55;
				buff[1]=0x06;
				buff[2]=0x29;
				buff[3]=0x00;
				buff[4]=0x80;
				buff[5]=0x02;
				buff[6]=0xff&minX;
				buff[7]=0xff&(minX>>8);
				for (i=0;i<8;i++)
				{
					spi_write(privRef->client,(unsigned char *)&buff[i],1);
				}
				msleep(5);
				eeprom_minX=minX;
				eeprom_write_mode[0]=0;
			}
	}
	else if (strcmp(attr->attr.name, "maxX") == 0)
	{
		maxX = calibrationSetting;
		if(calibrationSetting>0 && calibrationSetting<4095)
			if(eeprom_maxX!=maxX)
			{
				unsigned char buff[16]={0}; 	
					
				eeprom_write_mode[2]=1;
				buff[0]=0x55;
				buff[1]=0x06;
				buff[2]=0x29;
				buff[3]=0x00;
				buff[4]=0x84;
				buff[5]=0x02;
				buff[6]=0xff&maxX;
				buff[7]=0xff&(maxX>>8);
				for (i=0;i<8;i++)
				{
					spi_write(privRef->client,(unsigned char *)&buff[i],1);
				}
				msleep(5);
				eeprom_maxX = maxX;
				eeprom_write_mode[2]=0;
			}
	}
	else if (strcmp(attr->attr.name, "minY") == 0)
	{
		minY = calibrationSetting;
		if(calibrationSetting>0 && calibrationSetting<4095)
			if(eeprom_minY!=minY)
			{
				unsigned char buff[16]={0}; 	
				
				eeprom_write_mode[1]=1;
				buff[0]=0x55;
				buff[1]=0x06;
				buff[2]=0x29;
				buff[3]=0x00;
				buff[4]=0x82;
				buff[5]=0x02;
				buff[6]=0xff&minY;
				buff[7]=0xff&(minY>>8);
				for (i=0;i<8;i++)
				{
					spi_write(privRef->client,(unsigned char *)&buff[i],1);
				}
				msleep(5);
				eeprom_minY=minY;
				eeprom_write_mode[1]=0;
			}
	}
	else if (strcmp(attr->attr.name, "maxY") == 0)
	{
		maxY = calibrationSetting;
		if(calibrationSetting>0 && calibrationSetting<4095)
			if(eeprom_maxY!=maxY)
			{
				unsigned char buff[16]={0}; 	
					
				eeprom_write_mode[3]=1;
				buff[0]=0x55;
				buff[1]=0x06;
				buff[2]=0x29;
				buff[3]=0x00;
				buff[4]=0x86;
				buff[5]=0x02;
				buff[6]=0xff&maxY;
				buff[7]=0xff&(maxY>>8);
				for (i=0;i<8;i++)
				{
					spi_write(privRef->client,(unsigned char *)&buff[i],1);
				}
				msleep(5);
				eeprom_maxY=maxY;
				eeprom_write_mode[3]=0;
			}
	}
	else if (strcmp(attr->attr.name, "invertX") == 0)
		invertX = calibrationSetting;
	else if (strcmp(attr->attr.name, "invertY") == 0)
	{
		if(minX!=0 && minY!=0 && maxX!=4095 && maxY !=4095)
		{
			unsigned char buff[16]={0}; 	

			invertY = calibrationSetting;
			eeprom_write_mode[4]=1;
			buff[0]=0x55;
			buff[1]=0x06;
			buff[2]=0x29;
			buff[3]=0x00;
			buff[4]=0x88;
			buff[5]=0x02;
			buff[6]=swapAxes?0x04:0x00;
			buff[6]|=invertX?0x02:0x00;
			buff[6]|=invertY?0x01:0x00;
			buff[7]=buff[6];
			for (i=0;i<8;i++)
			{
				spi_write(privRef->client,(unsigned char *)&buff[i],1);
			}
			//msleep(5);
			eeprom_write_mode[4]=0;
		}
	}
#endif
	return count;
}

/* Defines sysfs variable associations */
#ifdef FIVE_POINT_CALIBRATION
static struct kobj_attribute Xorg_attribute =
	__ATTR(Xorg, 0660, calibrationSettings_show, calibrationSettings_store);
static struct kobj_attribute Yorg_attribute =
	__ATTR(Yorg, 0660, calibrationSettings_show, calibrationSettings_store);
static struct kobj_attribute XLorg_attribute =
	__ATTR(XLorg, 0660, calibrationSettings_show, calibrationSettings_store);
static struct kobj_attribute YLorg_attribute =
	__ATTR(YLorg, 0660, calibrationSettings_show, calibrationSettings_store);
static struct kobj_attribute XFactory_attribute =
	__ATTR(XFactory, 0660, calibrationSettings_show, calibrationSettings_store);
static struct kobj_attribute YFactory_attribute =
	__ATTR(YFactory, 0660, calibrationSettings_show, calibrationSettings_store);
#else
static struct kobj_attribute minX_attribute =
	__ATTR(minX, 0660, calibrationSettings_show, calibrationSettings_store);
static struct kobj_attribute maxX_attribute =
	__ATTR(maxX, 0660, calibrationSettings_show, calibrationSettings_store);
static struct kobj_attribute minY_attribute =
	__ATTR(minY, 0660, calibrationSettings_show, calibrationSettings_store);
static struct kobj_attribute maxY_attribute =
	__ATTR(maxY, 0660, calibrationSettings_show, calibrationSettings_store);
static struct kobj_attribute invertX_attribute =
	__ATTR(invertX, 0660, calibrationSettings_show, calibrationSettings_store);
static struct kobj_attribute invertY_attribute =
	__ATTR(invertY, 0660, calibrationSettings_show, calibrationSettings_store);
#endif
static struct kobj_attribute swapAxes_attribute =
	__ATTR(swapAxes, 0660, calibrationSettings_show, calibrationSettings_store);
static struct kobj_attribute lastPUCoordX_attribute =
	__ATTR(lastPUCoordX, 0660, calibrationSettings_show, calibrationSettings_store);
static struct kobj_attribute lastPUCoordY_attribute =
	__ATTR(lastPUCoordY, 0660, calibrationSettings_show, calibrationSettings_store);

/*
 * Create a group of calibration attributes so we may work with them
 * as a set.
 */
static struct attribute *attrs[] = {
	&commandDataPending_attribute.attr,
	&commandMode_attribute.attr,
	&receiveBuffer_attribute.attr,
	&sendBuffer_attribute.attr,
#ifdef FIVE_POINT_CALIBRATION
	&Xorg_attribute.attr,
	&Yorg_attribute.attr,
	&XLorg_attribute.attr,
	&YLorg_attribute.attr,
	&XFactory_attribute.attr,
	&YFactory_attribute.attr,
#else
	&minX_attribute.attr,
	&maxX_attribute.attr,
	&minY_attribute.attr,
	&maxY_attribute.attr,
	&invertX_attribute.attr,
	&invertY_attribute.attr,
#endif
	&swapAxes_attribute.attr,
	&lastPUCoordX_attribute.attr,
	&lastPUCoordY_attribute.attr,
	NULL,	
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

static struct kobject *ar1021_kobj;


static irqreturn_t touch_irq_handler_func(int irq, void *dev_id);

/******************************************************************************
Function:
	decodear1021Packet()

Description:
	Decode packets of data from a device path using AR1XXX protocol. 
	Returns 1 if a full packet is available, zero otherwise.
******************************************************************************/
int decodeAR1021Packet(struct ar1021_spi_priv* priv, char* packet, int *index, char data)
{
	int returnValue=0;
	int x;
	int y;
	int button;
	int calX=-1;
	int calY=-1;
	
	packet[*index] = data;

	/****************************************************
	
	Data format, 5 bytes: SYNC, DATA1, DATA2, DATA3, DATA4
	
	SYNC [7:0]: 1,0,0,0,0,TOUCHSTATUS[0:0]
	DATA1[7:0]: 0,X-LOW[6:0]
	DATA2[7:0]: 0,X-HIGH[4:0]
	DATA3[7:0]: 0,Y-LOW[6:0]
	DATA4[7:0]: 0,Y-HIGH[4:0]
	
	TOUCHSTATUS: 0 = Touch up, 1 = Touch down
	
	****************************************************/		
	
	switch ((*index)++) {
		case 0:
			if (!(0x80 & data))
			{
				//printk("AR1021 SPI: Sync bit not set 0x%02x\n",data);
				/* Sync bit not set */
				*index=0;
				returnValue=-1;
			}
			break;

		case (MCHIP_MAX_LENGTH - 1):
			/* verify byte is valid for current index */
			if (0x80 & data)
			{
				//printk("AR1021 SPI: Byte not valid 1 0x%02x\n",data);
				/* byte not valid */
				*index=1;
				break;
			}		  

			x = ((packet[2] & 0x1f) << 7) | (packet[1] & 0x7f);
			y = ((packet[4] & 0x1f) << 7) | (packet[3] & 0x7f);
			button = 0 != (packet[0] & 1);

			if (0==button)
			{
				lastPUCoordX=x;
				lastPUCoordY=y;
			}

			if (swapAxes)
			{
				int temp=x;
				x=y;
				y=temp;
			}
#ifndef CONFIG_ANDROID
			input_report_abs(priv->input, ABS_X, x);
			input_report_abs(priv->input, ABS_Y, y);
			input_report_key(priv->input, BTN_TOUCH, button);
			input_sync(priv->input);
			return 1;
#endif
			if(XFactory==0 || YFactory ==0)
			{
#ifndef FIVE_POINT_CALIBRATION

				if (invertX)
					x=4095-x;

				if (invertY)
					y=4095-y;

				if (x<minX)
				{
					button=0;
					calX=0;
				}
				else if (x>maxX)
				{
					button=0;
					calX=4095;
				}
				else
					/* percentage across calibration area times the maximum controller width */
					calX=((x-minX)*4095)/(maxX-minX);

				if (y<minY)
				{
					button=0;
					calY=0;
				}
				else if (y>maxY)
				{
					button=0;
					calY=4095;
				}
				else
					/* percentage across calibration area times the maximum controller height */
					calY=((y-minY)*4095)/(maxY-minY);
#endif
				//printk("ar1021 (%d,%d)%d(%d,%d)\n",calX,calY,button, x, y);
				if(button)
				{
					input_report_abs(priv->input, ABS_MT_POSITION_X, calX==-1?x:calX);
					input_report_abs(priv->input, ABS_MT_POSITION_Y, calY==-1?y:calY);
					input_report_abs(priv->input, ABS_MT_PRESSURE, 127);
					input_report_abs(priv->input, ABS_MT_TRACKING_ID, 0);
					input_report_abs(priv->input, ABS_MT_TOUCH_MAJOR, 127);
				}
				//input_sync(priv->input);				
				input_report_key(priv->input, BTN_TOUCH, button);
				input_sync(priv->input);
			}
			else
			{
				calX=(x-XLorg)*10000/XFactory+Xorg;
				if(calX>4095)
					calX=4095;
				else if(calX<0)
					calX=0;
				calY=(y-YLorg)*10000/YFactory+Yorg;
				if (calY>4095)
					calY=4095;
				else if (calY<0)
					calY=0;
				if(button)
				{
					input_report_abs(priv->input, ABS_MT_POSITION_X, calX);
					input_report_abs(priv->input, ABS_MT_POSITION_Y, calY);
					input_report_abs(priv->input, ABS_MT_PRESSURE, 127);
					input_report_abs(priv->input, ABS_MT_TRACKING_ID, 0);
					input_report_abs(priv->input, ABS_MT_TOUCH_MAJOR, 127);
				}
				//input_sync(priv->input);				
				input_report_key(priv->input, BTN_TOUCH, button);
				input_sync(priv->input);
				//printk("ar1021 (%d,%d)%d(%d,%d)\n",calX,calY,button, x, y);
			}
			returnValue=1;
			break;
		default:
			/* verify byte is valid for current index */
			if (0x80 & data)
			{
				/* byte not valid */
				//printk("AR1021 SPI: Byte not valid. Value: 0x%02x Index: 0x%02x\n",data, *index);
				*index=1;
				returnValue=-1;
			}			  
			break;
			
	}

	return returnValue;
}

/******************************************************************************
Function:
	ar1021_spi_open()

Description:
	This function is called on every attempt to open the current device  
	and used for both debugging purposes fullfilling an SPI driver 
	function callback requirement.
******************************************************************************/
static int ar1021_spi_open(struct input_dev *dev)
{
	return 0;
}

/******************************************************************************
Function:
	ar1021_spi_close()

Description:
	This function is called on every attempt to close the current device  
	and used for both debugging purposes fullfilling an SPI driver 
	function callback requirement.
******************************************************************************/
static void ar1021_spi_close(struct input_dev *dev)
{
}

/******************************************************************************
Function:
	test_irq_handler_func()

Description:
	Testing to see if IRQ line of controller attached to an available 
	IO line on board.
******************************************************************************/
static irqreturn_t test_irq_handler_func(int irq, void *dev_id)
{
	struct ar1021_spi_priv *priv = (struct ar1021_spi_priv *)dev_id;
	int err;

	if (!priv) {
		printk(KERN_ERR "ar1021 SPI: touch_irq_handler_funct: no private data\n");
		err = -EINVAL;
		return err;
	}

	priv->testCount++;

	return IRQ_NONE;
}

static void ar1021_spi_eepromdata(struct work_struct *work,int eeprom_offset)
{
	struct ar1021_spi_priv *priv =
		container_of(work, struct ar1021_spi_priv, work);
	char buff[16];
	int i=0;
	msleep(1);
	buff[0]=0x55;
	buff[1]=0x04;
	buff[2]=0x28;
	buff[3]=0x00;
#ifdef FIVE_POINT_CALIBRATION
	buff[4]=0x90+2*(0x0f&eeprom_offset);
#else
	buff[4]=0x80+2*(0x0f&eeprom_offset);
#endif
	buff[5]=0x02;
	for (i=0;i<6;i++)
	{
		//printk(KERN_DEBUG "0x%02x ",buff[i]);
		spi_write(priv->client,(unsigned char *)&buff[i],1);
	}
}

/******************************************************************************
Function:
	ar1021_spi_readdata()

Description:
	When the controller interrupt is asserted, this function is scheduled
	to be called to read the controller data within the 
	touch_irq_handler_func() function.
******************************************************************************/
static void ar1021_spi_readdata(struct work_struct *work)
{
	struct ar1021_spi_priv *priv =
		container_of(work, struct ar1021_spi_priv, work);
	int index=0;
	char buff[16];
	int ret;
	int i;

	memset(buff,0x0,sizeof(buff));
	/* We want to ensure we only read packets when we are not in the middle of command communication. Disable command mode after receiving command response to resume receiving packets. */
	if (commandMode)
	{
		commandDataPending=1;
		/* process up to 9 bytes */
		strcpy(receiveBuffer,"");

		/* header byte */
		spi_read(priv->client,&buff[0],1);
		snprintf(receiveBuffer,sizeof(receiveBuffer),"0x%02x",0xff&buff[0]);

		if (0x55 != buff[0])
		{
			printk("ar1021 SPI: invalid header byte\n");
			return;
		}

		/* num data bytes */
		spi_read(priv->client,&buff[1],1);
		snprintf(receiveBuffer,sizeof(receiveBuffer),"%s 0x%02x",receiveBuffer,0xff&buff[1]);
		if (buff[1] >0x0A)
		{
			printk("ar1021 SPI: invalid byte count\n");
			return;
		}
		
		for (i=0;i<buff[1];i++)
		{
			spi_read(priv->client,&buff[i+2],1);
			snprintf(receiveBuffer,sizeof(receiveBuffer),"%s 0x%02x",receiveBuffer,0xff&buff[i+2]);
		}
		snprintf(receiveBuffer,sizeof(receiveBuffer),"%s\n",receiveBuffer);
		printk( "ar1021 SPI: command response: %s",receiveBuffer);
		return;
	}

	for(i=0;i<6;i++)
		if(eeprom_write_mode[i])
			return;

	if(eeprom_read_mode_flag)
	{
		/* header byte */
		spi_read(priv->client,&buff[0],1);
		if (0x55 != buff[0])
		{
			printk("ar1021 SPI: invalid header byte eeprom_read_mode_flag\n");
			return;
		}

		/* num data bytes */
		spi_read(priv->client,&buff[1],1);
		if (buff[1] >0x0A)
		{
			printk("ar1021 SPI: invalid byte count\n");
			return;
		}
		
		for (i=0;i<buff[1];i++)
		{
			spi_read(priv->client,&buff[i+2],1);
		}
			
#ifdef FIVE_POINT_CALIBRATION
		switch(eeprom_read_mode_flag)
		{
			case 1:
				if((buff[5]&0x80)==0x80)
				{
					buff[5]&=0x7f;
					ret=-1*(buff[4]+256*buff[5]);
				}
				else
				{
					ret=buff[4]+256*buff[5];
				}
				if(ret<-20000 || ret >20000  || ret==0)
					goto five_point_error_return;
				eeprom_XFactory=XFactory=ret;
				printk("ar1020 SPI(%d): XFactory: %d",eeprom_read_mode_flag,XFactory);
				break;
			case 2:
				if((buff[5]&0x80)==0x80)
				{
					buff[5]&=0x7f;
					ret=-1*(buff[4]+256*buff[5]);
				}
				else
				{
					ret=buff[4]+256*buff[5];
				}
				if(ret<-20000 || ret >20000 || ret==0)
					goto five_point_error_return;
				eeprom_YFactory=YFactory=ret;
				printk("ar1020 SPI(%d): YFactory: %d",eeprom_read_mode_flag,YFactory);
				break;
			case 3:
				if((buff[5]&0x10)==0x10)
				{
					buff[5]&=0x0f;
					swapAxes=1;
				}
				else
					swapAxes=0;
				ret=buff[4]+256*buff[5];
				if(ret<0 || ret >4095)
					goto five_point_error_return;
				eeprom_XLorg=XLorg=ret;
				printk("ar1020 SPI(%d): XLorg: %d",eeprom_read_mode_flag,XLorg);
				break;
			case 4:
				if((buff[5]&0x10)==0x10)
				{
					buff[5]&=0x0f;
					swapAxes=1;
				}
				else
					swapAxes=0;
				ret=buff[4]+256*buff[5];
				if(ret<0 || ret >4095)
					goto five_point_error_return;
				eeprom_YLorg=YLorg=ret;
				printk("ar1020 SPI(%d): YLorg: %d",eeprom_read_mode_flag,YLorg);
				break;
			case 5:
				if((buff[5]&0x10)==0x10)
				{
					buff[5]&=0x0f;
					swapAxes=1;
				}
//				else
//					swapAxes=0;
				ret=buff[4]+256*buff[5];
				if(ret<0 || ret >4095)
					goto five_point_error_return;
				eeprom_Xorg=Xorg=ret;
				printk("ar1020 SPI(%d): Xorg: %d",eeprom_read_mode_flag,Xorg);
				break;
			case 6:
				if((buff[5]&0x10)==0x10)
				{
					buff[5]&=0x0f;
					swapAxes=1;
				}
//				else
//					swapAxes=0;
				ret=buff[4]+256*buff[5];
				if(ret<0 || ret >4095)
					goto five_point_error_return;
				eeprom_Yorg=Yorg=ret;
				printk("ar1020 SPI(%d): Yorg: %d",eeprom_read_mode_flag,Yorg);
				eeprom_read_mode_flag=0;
				return;
				break;
		}
		ar1021_spi_eepromdata(work,eeprom_read_mode_flag);
		eeprom_read_mode_flag++;	
		return;
five_point_error_return:
		printk("ar1020 SP(%d)I: five point data error: %d",eeprom_read_mode_flag,ret);
		eeprom_read_mode_flag=0;
		XFactory=YFactory=0;
		return;
#else
		ret=buff[4]+256*buff[5];
		if(ret<0 || ret >4095)
		{
			printk("ar1021 SPI: data error(%d): %d",eeprom_read_mode,ret);
			eeprom_read_mode_flag=0;
			return;
		}
		switch(eeprom_read_mode_flag)
		{
			case 1:
				eeprom_minX=ret;
				printk("ar1020 SPI(%d): eeprom_minX: %d",eeprom_read_mode_flag,eeprom_minX);
				break;
			case 2:
				eeprom_minY=ret;
				printk("ar1020 SPI(%d): eeprom_minY: %d",eeprom_read_mode_flag,eeprom_minY);
				break;
			case 3:
				eeprom_maxX=ret;
				printk("ar1020 SPI(%d): eeprom_maxX: %d",eeprom_read_mode_flag,eeprom_maxX);
				break;
			case 4:
				eeprom_maxY=ret;
				printk("ar1020 SPI(%d): eeprom_maxY: %d",eeprom_read_mode_flag,eeprom_maxY);
				break;
			case 5:
				if(buff[4]==buff[5] && buff[4]<=0x07)
				{
					minX=eeprom_minX;
					maxX=eeprom_maxX;
					minY=eeprom_minY;
					maxY=eeprom_maxY;
					swapAxes	=	eeprom_swapAxes	=	buff[4]&0x04?1:0;				
					invertX	=	eeprom_invertX	=	buff[4]&0x02?1:0;				
					invertY	=	eeprom_invertY	=	buff[4]&0x01?1:0;
				}
				else
					printk("ar1020 SPI: eeprom_read_mode_flag(%d) error",eeprom_read_mode_flag);
				eeprom_read_mode_flag=0;
				return;
				break;
				
		}
		ar1021_spi_eepromdata(work,eeprom_read_mode_flag);
		eeprom_read_mode_flag++;
		return;
#endif
	}
	
#ifdef spi_print_debug
	printk("ar1021 :");
	/* process up to 9 bytes */
	for (i=0;i<5;i++)
	{
		spi_read(priv->client,&buff[index],1);
		printk(" 0x%02x",buff[index]);
	}
	printk("\n");
#else
	/* process up to 9 bytes */
	for (i=0;i<9;i++)
	{
		spi_read(priv->client,&buff[index],1);
		ret=decodeAR1021Packet(priv,buff, &index, buff[index]);
		/* if a one is returned, then we have a full packet */
		if (ret==1 || index >4)
		{
			break;
		}	
	}
#endif
}

/******************************************************************************
Function:
	ar1021_spi_probe()

Description:
	After the kernel's platform specific source files have been modified to 
	reference the "ar1021_spi" driver, this function will then be called.
	This function needs to be called to finish registering the driver.
******************************************************************************/
static int ar1021_spi_probe(struct spi_device *client)
{
	struct ar1021_spi_priv *priv=NULL;
	struct input_dev *input_dev=NULL;
	int err=0;
	int i;
	int j;
	unsigned char buff[16];
	int ret;

	printk(KERN_ERR "SPI CLK %d Hz?\n", client->max_speed_hz);
	for (i=0;i<5;i++)
	{
		buff[i]=0;
	}

	if (!client) {
		printk(KERN_ERR "ar1021 SPI: client pointer is NULL\n");
		err = -EINVAL;
		goto error;
	}

	if ((!client->irq) && (touchIRQ == -1) && (!testSPIdata) && (!probeForIRQ)) {
		printk(KERN_ERR "ar1021 SPI: no IRQ set for touch controller\n");
		err = -EINVAL;
		goto error;
	}

	priv = kzalloc(sizeof(struct ar1021_spi_priv), GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!priv) {
		printk(KERN_ERR "ar1021 SPI: kzalloc error\n");
		err = -ENOMEM;
		goto error;
	}

	/* Backup pointer so sysfs helper functions may also have access to private data */
	privRef=priv;

	if (!input_dev)
	{
		printk(KERN_ERR "ar1021 SPI: input allocate error\n");
		err = -ENOMEM;		
		goto error;
	}


	priv->client = client;
	priv->irq = client->irq;
	priv->input = input_dev;

	/* Verify raw SPI data stream to ensure bus is setup correctly in the platform settings. */
	if (testSPIdata)
	{
		printk("ar1021 SPI: In testing mode to verify packet.  To inhibit this mode,\n");
		printk("unset the \"testSPIdata\" kernel parameter.\n");
		while (1)
		{
			msleep(1);
			spi_read(priv->client,&buff[0],1);

  		        if (!(0x80 & buff[0]))
			{				
				if ((buff[0]!= 0x4d) && (buff[0]!=0x00))
				{
					printk("0x%02x ",buff[0]);
				}
			}
			else
			{
				printk("\n0x%02x ",buff[0]);
				for (i=1;i<5;i++)
				{
					spi_read(priv->client,&buff[i],1);
					printk("0x%02x ",buff[i]);
				}
				printk("\n");
			}		

		}		

	}

	/* Detect IRQ id that controller IRQ line is attached to.  This detection only works
	   if the controller's IRQ line is attached to a GPIO line configured as an input.
	   These lines are often marked as EINT (external interrupt) on the board schematic. 
	   This probe assumes that SPI read communication with the controller is working 
	   correctly.
	*/ 
	if (probeForIRQ)
	{
		printk("ar1021 SPI: Probing for interrupt id.\n");
		printk("ar1021 SPI: Please touch screen before IRQ probe for successful detection.\n");
		printk("ar1021 SPI: Probing will commence in five seconds.\n\n");
		printk("ar1021 SPI: Kernel exception messages may appear during the\n");
		printk("ar1021 SPI: probing process.\n");

		msleep(5000);
		for (i=probeMin;i<probeMax;i++)
		{
			printk("ar1021 SPI: Testing IRQ %d\n",i);
			priv->irq=i;

			/* set type on new handler and register gpio pin as our interrupt */
			//danny modify

			//set_irq_type(i, IRQ_TYPE_EDGE_RISING);
			//err = request_threaded_irq(client->irq, NULL,
			//      ft5x06_ts_interrupt, IRQF_TRIGGER_FALLING,
			//      client->dev.driver->name, data);


//			if (0 >= (ret=request_threaded_irq(i, NULL,
//					test_irq_handler_func,IRQF_TRIGGER_RISING, "ar1021 IRQ", priv)))
			if (0 >= (ret=request_irq(i, test_irq_handler_func,IRQF_TRIGGER_RISING, "ar1021 IRQ", priv)))
			{
				priv->testCount=0;

				/* read SPI data to ensure IRQ line is not asserted */
				for (j=0;j<5;j++)
				{
				  spi_read(priv->client,&buff[j],1);
				}

				msleep(1000);
				if (ret>=0)
				{
					free_irq(i, priv);
				}

				/* successful detection if count within this range */
				if ((priv->testCount > 0) && (priv->testCount < 3))
				{
					printk("ar1021 SPI: Touch IRQ detected at ID: %d.\n",i);
					priv->irq=i;
					break;
				}
			}
			else
			{
				printk("ar1021 SPI: IRQ %d not available.\n", i);
			}
	  	}
		if (i==probeMax)
		{
			printk("ar1021 SPI: Touch IRQ not detected. Using IRQ %d.\n",priv->irq);
		}

	}
	/* Use default settings */
	else if (touchIRQ == -1)
	{
		printk("ar1021 SPI: Using IRQ %d set via board's platform setting.\n", priv->irq);		
	}
	else
	{
		printk("ar1021 SPI: Using IRQ %d set via kernel parameter\n", touchIRQ);
		priv->irq=touchIRQ;
	}



	INIT_WORK(&priv->work, ar1021_spi_readdata);

	input_dev->name = "ar1021 Touchscreen";

	input_dev->open = ar1021_spi_open;
	input_dev->close = ar1021_spi_close;

	//set_bit(KEY_HOME, input_dev->keybit);  
	//set_bit(KEY_SEARCH, input_dev->keybit);  
	//set_bit(KEY_BACK, input_dev->keybit);  
	//set_bit(KEY_MENU, input_dev->keybit); 
#ifdef CONFIG_ANDROID
	__set_bit(EV_KEY, input_dev->evbit);
	__set_bit(EV_ABS, input_dev->evbit);
	__set_bit(BTN_TOUCH, input_dev->keybit);
	__set_bit(INPUT_PROP_DIRECT, input_dev->propbit);
	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, 4095, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, 4095, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0, 1, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, 0X7F, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_PRESSURE, 0, 0x7f, 0, 0);
#else
	input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
	input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);

	input_set_abs_params(input_dev, ABS_X, 0, 4095, 0, 0);
	input_set_abs_params(input_dev, ABS_Y, 0, 4095, 0, 0);
#endif
	err = input_register_device(input_dev);
	if (err)
	{
		printk(KERN_ERR "ar1021 SPI: error registering input device\n");
		goto error;
	}

	for (i=0;i<5;i++)
	{
		spi_read(priv->client,&buff[i],1);
	}
	for (i=0;i<5;i++)
	{
		eeprom_write_mode[i]=0;
	}

	/*
	and enter the following commands: 
	echo "1">/sys/kernel/ar1020/commandMode
	echo "0x55 0x05 0x21 0x00 0x24 0x01 0x10">/sys/kernel/ar1021/sendBuffer
	echo "0x55 0x05 0x21 0x00 0x25 0x01 0x80">/sys/kernel/ar1021/sendBuffer
	echo "0">/sys/kernel/ar1021/commandMode
	echo "0x55 0x04 0x20 0x00 0x24 0x04">/sys/kernel/ar1021/sendBuffer
	echo "0x55 0x04 0x28 0x00 0x90 0x04">/sys/kernel/ar1020/sendBuffer
	echo "0x55 0x04 0x28 0x00 0x94 0x04">/sys/kernel/ar1020/sendBuffer
	*/
#ifdef CHANGE_DEFAULT_SETTING
	//		0x23	0x24	0x25	0x26	0x27
	//		0x04	0x04	0x10	0x04	0x08
	msleep(2);
	buff[0]=0x55;
	buff[1]=0x06;
	buff[2]=0x21;
	buff[3]=0x00;
	buff[4]=0x24;
	buff[5]=0x02;
	buff[6]=0x10;
	buff[7]=0x80;
	for (i=0;i<8;i++)
	{
		//printk(KERN_DEBUG "0x%02x ",buff[i]);
		spi_write(priv->client,(unsigned char *)&buff[i],1);
	}
#endif
	msleep(2);
	ret = request_threaded_irq(priv->irq, NULL, touch_irq_handler_func,  IRQF_TRIGGER_RISING | IRQF_ONESHOT,  "ar1021_spi", priv);
	if (ret) {
		dev_err(&client->dev, "failed to request IRQ %d, err: %d\n",
			priv->irq, ret);
		goto error;
	}
#ifndef FIVE_POINT_CALIBRATION
	eeprom_read_mode_flag=1;
	msleep(1);
	buff[0]=0x55;
	buff[1]=0x04;
	buff[2]=0x28;
	buff[3]=0x00;
	buff[4]=0x80;
	buff[5]=0x02;
	for (i=0;i<6;i++)
	{
		spi_write(priv->client,(unsigned char *)&buff[i],1);
	}
#else
	eeprom_read_mode_flag=1;
	msleep(1);
	buff[0]=0x55;
	buff[1]=0x04;
	buff[2]=0x28;
	buff[3]=0x00;
	buff[4]=0x90;
	buff[5]=0x02;
	for (i=0;i<6;i++)
	{
		spi_write(priv->client,(unsigned char *)&buff[i],1);
	}
#endif
	return 0;

 error:

	if (input_dev)
		input_free_device(input_dev);

	if (priv)
		kfree(priv);

	return err;

	return 0;
}

/******************************************************************************
Function:
	ar1021_spi_remove()

Description:
	Unregister/remove the kernel driver from memory. 
******************************************************************************/
static int ar1021_spi_remove(struct spi_device *client)
{
	struct ar1021_spi_priv *priv = (struct ar1021_spi_priv *)dev_get_drvdata(&client->dev);

	printk("ar1021 SPI: ar1021_spi_remove: begin\n");

	free_irq(priv->irq, priv);
	input_unregister_device(priv->input);
	kfree(priv);

	return 0;
}

/******************************************************************************
Function:
	touch_irq_handler_func()

Description:
	After the interrupt is asserted for the controller, this
	is the first function that is called.  Since this is a time sensitive
	function, we need to immediately schedule work so the integrity of
	properly system operation 

	This function needs to be called to finish registering the driver.
******************************************************************************/
static irqreturn_t touch_irq_handler_func(int irq, void *dev_id)
{
	struct ar1021_spi_priv *priv = (struct ar1021_spi_priv *)dev_id;
	char buff[5];
	int i;
	int err;
	for (i=0;i<5;i++)
	{
		buff[i]=0;	  
	}

	//printk("<danny debug> ar1021 interrupt up\n");
	if (!priv) {
		printk(KERN_ERR "ar1021 SPI: touch_irq_handler_funct: no private data\n");
		err = -EINVAL;
		return err;
	}

	 /* delegate SPI transactions since hardware interupts need to be handled very fast */
	schedule_work(&priv->work);

	return IRQ_HANDLED;
}

/* This is the initial set of information information the kernel has
   before probing drivers on the system, */
#if 0
static struct spi_driver ar1021_spi_driver = {
	.driver = {
		.name	= "ar1021-spi",
		.bus	= &spi_bus_type,
		.owner	= THIS_MODULE,
	},
	.probe		= ar1021_spi_probe,
	.remove		= ar1021_spi_remove,
	/* suspend/resume functions not needed since controller automatically
  	   put's itself to sleep mode after configurable short period of time */
	.suspend	= ar1021_suspend,
	.resume		= ar1021_resume,
};
#endif
//static const struct spi_device_id ar1021_spi_id[] = {
//	{ "MICROCHIP_AR1021_SPI", 0 },
//	{ },
//};
//MODULE_DEVICE_TABLE(i2c, ar1021_spi_id);
static SIMPLE_DEV_PM_OPS(ar1021_pm_ops, ar1021_suspend, ar1021_resume);

static const struct of_device_id ar1021_spi_of_match[] = {
	{ .compatible = "microchip,ar1021-spi", },
	{ }
};
MODULE_DEVICE_TABLE(of, ar1021_spi_of_match);
static struct spi_driver ar1021_spi_driver = {
	.driver = {
		.name	= "ar1021_spi",
		.owner	= THIS_MODULE,
		.pm	= &ar1021_pm_ops,
		.of_match_table = ar1021_spi_of_match,
	},
	.probe		= ar1021_spi_probe,
	.remove		= ar1021_spi_remove,
};
#if 1
/******************************************************************************
Function:
	ar1021_spi_init()

Description:
	This function is called during startup even if the platform specific
	files have not been setup yet.
******************************************************************************/
static int __init ar1021_spi_init(void)
{
	int retval;

        printk("ar1021 SPI: ar1021_spi_init: begin\n");
	strcpy(receiveBuffer,"");
	strcpy(sendBuffer,"");

	/*
	 * Creates a kobject "ar1021" that appears as a sub-directory
	 * under "/sys/kernel".
	 */
	ar1021_kobj = kobject_create_and_add("ar1021", kernel_kobj);
	if (!ar1021_kobj)
	{
		printk(KERN_ERR "ar1021 SPI: cannot create kobject\n");
		return -ENOMEM;
	}

	/* Create the files associated with this kobject */
	retval = sysfs_create_group(ar1021_kobj, &attr_group);
	if (retval)
	{
		printk(KERN_ERR "ar1021 SPI: error registering ar1021-spi driver's sysfs interface\n");
		kobject_put(ar1021_kobj);
	}

	return spi_register_driver(&ar1021_spi_driver);
}

/******************************************************************************
Function:
	ar1021_spi_exit()

Description:
	This function is called after ar1021_spi_remove() immediately before 
	being removed from the kernel.
******************************************************************************/
static void __exit ar1021_spi_exit(void)
{
	printk("ar1021 SPI: ar1021_i2c_exit begin\n");
	kobject_put(ar1021_kobj);
	spi_unregister_driver(&ar1021_spi_driver);
}
#endif
MODULE_AUTHOR("leelin@aplex.com.tw");
MODULE_DESCRIPTION("ar1021 touchscreen SPI bus driver");
MODULE_LICENSE("GPL");
//module_spi_driver(ar1021_spi_driver);

/* Enable the ar1021_spi_init() to be run by the kernel during initialization */
module_init(ar1021_spi_init);

/* Enables the ar1021_spi_exit() to be called during cleanup.  This only
has an effect if the driver is compiled as a kernel module. */
module_exit(ar1021_spi_exit);

