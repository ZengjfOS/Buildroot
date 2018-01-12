#include <linux/module.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>
//#include <mach/iomux-mx6dl.h>
//#define IMX_GPIO_NR(bank, nr)		(((bank) - 1) * 32 + (nr))
/**
	//GPIO write by zengjf
	MX6DL_PAD_SD2_DAT0__GPIO_1_15,	//GPIO In
	MX6DL_PAD_SD2_DAT1__GPIO_1_14,
	MX6DL_PAD_SD2_DAT2__GPIO_1_13,
	MX6DL_PAD_SD2_DAT3__GPIO_1_12,
	MX6DL_PAD_NANDF_D4__GPIO_2_4,	//GPIO Out
	MX6DL_PAD_NANDF_D5__GPIO_2_5,
	MX6DL_PAD_NANDF_D6__GPIO_2_6,
	MX6DL_PAD_NANDF_D7__GPIO_2_7
 */
#define GPIO_IN0_mio				IMX_GPIO_NR(1, 15)
#define GPIO_IN1_mio				IMX_GPIO_NR(1, 14)
#define GPIO_IN2_mio				IMX_GPIO_NR(1, 13)
#define GPIO_IN3_mio				IMX_GPIO_NR(1, 12)
#define GPIO_OUT0_mio				IMX_GPIO_NR(2, 4)
#define GPIO_OUT1_mio				IMX_GPIO_NR(2, 5)
#define GPIO_OUT2_mio				IMX_GPIO_NR(2, 6)
#define GPIO_OUT3_mio				IMX_GPIO_NR(2, 7)

#define GPIO_IN0_CMD_mio			66
#define GPIO_IN1_CMD_mio			67
#define GPIO_IN2_CMD_mio			68
#define GPIO_IN3_CMD_mio			69
#define GPIO_OUT0_CMD_mio			70
#define GPIO_OUT1_CMD_mio			71
#define GPIO_OUT2_CMD_mio			72
#define GPIO_OUT3_CMD_mio			73


static int gpio_mio_open(struct inode *inode, struct file *file)
{
    //printk("gpio mio open.\n")
	int ret;
	ret = gpio_request(GPIO_IN0_mio, "gpio_in0_mio");
	if ( ret ) {
        printk("get gpio_in0_mio gpio FAILED!\n");
		return ret;
	}

	ret = gpio_request(GPIO_IN1_mio, "gpio_in1_mio");
	if ( ret ) {
        printk("get gpio_in1_mio gpio FAILED!\n");
		return ret;
	}

	ret = gpio_request(GPIO_IN2_mio, "gpio_in2_mio");
	if ( ret ) {
        printk("get gpio_in2_mio gpio FAILED!\n");
		return ret;
	}

	ret = gpio_request(GPIO_IN3_mio, "gpio_in3_mio");
	if ( ret ) {
        printk("get gpio_in3_mio gpio FAILED!\n");
		return ret;
	}

	ret = gpio_request(GPIO_OUT0_mio, "gpio_out0_mio");
	if ( ret ) {
        printk("get gpio_out0_mio gpio FAILED!\n");
		return ret;
	}

	ret = gpio_request(GPIO_OUT1_mio, "gpio_out1_mio");
	if ( ret ) {
        printk("get gpio_out1_mio gpio FAILED!\n");
		return ret;
	}

	ret = gpio_request(GPIO_OUT2_mio, "gpio_out2_mio");
	if ( ret ) {
        printk("get gpio_out2_mio gpio FAILED!\n");
		return ret;
	}

	ret = gpio_request(GPIO_OUT3_mio, "gpio_out3_mio");
	if ( ret ) {
        printk("get gpio_out3_mio gpio FAILED!\n");
		return ret;
	}

	gpio_direction_input(GPIO_IN0_mio);
	gpio_direction_input(GPIO_IN1_mio);
	gpio_direction_input(GPIO_IN2_mio);
	gpio_direction_input(GPIO_IN3_mio);
	//set the default value
	gpio_direction_output(GPIO_OUT0_mio, 0);
	gpio_direction_output(GPIO_OUT1_mio, 0);
	gpio_direction_output(GPIO_OUT2_mio, 0);
	gpio_direction_output(GPIO_OUT3_mio, 0);

	printk("register gpio input output over.\n");

    return 0;
}

static int gpio_mio_close(struct inode *inode, struct file *file)
{
    //printk("Dev close.\n");
	//set back for default value
	gpio_direction_output(GPIO_IN0_mio, 0);
	gpio_direction_output(GPIO_IN1_mio, 0);
	gpio_direction_output(GPIO_IN2_mio, 0);
	gpio_direction_output(GPIO_IN3_mio, 0);
	gpio_direction_output(GPIO_OUT0_mio, 0);
	gpio_direction_output(GPIO_OUT1_mio, 0);
	gpio_direction_output(GPIO_OUT2_mio, 0);
	gpio_direction_output(GPIO_OUT3_mio, 0);
	gpio_free(GPIO_IN0_mio);
	gpio_free(GPIO_IN1_mio);
	gpio_free(GPIO_IN2_mio);
	gpio_free(GPIO_IN3_mio);
	gpio_free(GPIO_OUT0_mio);
	gpio_free(GPIO_OUT1_mio);
	gpio_free(GPIO_OUT2_mio);
	gpio_free(GPIO_OUT3_mio);

    return 0;
}

long gpio_ioctl(struct file * file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	
	switch (cmd) {
		case GPIO_IN0_CMD_mio  : 
			ret = gpio_get_value(GPIO_IN0_mio);
			//printk("test gpio in0 ret %d\n", ret);
			break;
		case GPIO_IN1_CMD_mio  : 
			ret = gpio_get_value(GPIO_IN1_mio);
			//printk("test gpio in1 ret %d\n", ret);
			break;
		case GPIO_IN2_CMD_mio  : 
			ret = gpio_get_value(GPIO_IN2_mio);
			//printk("test gpio in2 ret %d\n", ret);
			break;
		case GPIO_IN3_CMD_mio  : 
			ret = gpio_get_value(GPIO_IN3_mio);
			//printk("test gpio in3 ret %d\n", ret);
			break;
		case GPIO_OUT0_CMD_mio :
			gpio_set_value(GPIO_OUT0_mio, arg != 0 ? 1 : 0);
			//printk("test gpio out0 arg %d\n", (int)arg);
			break;
		case GPIO_OUT1_CMD_mio :
			gpio_set_value(GPIO_OUT1_mio, arg != 0 ? 1 : 0);
			//printk("test gpio out1 arg %d\n", (int)arg);
			break;
		case GPIO_OUT2_CMD_mio :
			gpio_set_value(GPIO_OUT2_mio, arg != 0 ? 1 : 0);
			//printk("test gpio out2 arg %d\n", (int)arg);
			break;
		case GPIO_OUT3_CMD_mio :
			gpio_set_value(GPIO_OUT3_mio, arg != 0 ? 1 : 0);
			//printk("test gpio out3 arg %d\n", (int)arg);
			break;
		default :
			printk("gpio_ioctl cmd failed.\n");

			return -1;
	}

	return ret;
}

struct file_operations gpio_fops = {
    .owner      = THIS_MODULE,
    .open       = gpio_mio_open,
    .release    = gpio_mio_close,
    //.read       = gpio_mio_read,
	.unlocked_ioctl = gpio_ioctl,
};

struct miscdevice gpio_misc = {
    .minor  = MISC_DYNAMIC_MINOR,
    .name   = "gpio_mio",
    .fops   = &gpio_fops,
};

int __init gpio_mio_init(void)
{
	int ret;

	ret = misc_register(&gpio_misc);
	if(ret)
		printk("gpio_misc_register FAILED!\n");
	else 
		printk("gpio mio has register\n");

	return ret;
}

void __exit gpio_mio_exit(void)
{
    misc_deregister(&gpio_misc);

}

module_init(gpio_mio_init);
module_exit(gpio_mio_exit);

MODULE_LICENSE("GPL");

