#include <linux/module.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>

#define SABRESD_WM8962_POWER   IMX_GPIO_NR(1, 30)

static int wm8962_ctrl_open(struct inode *inode, struct file *file)
{
    printk("Dev open.\n");

    return 0;
}

static int wm8962_ctrl_close(struct inode *inode, struct file *file)
{
    printk("Dev close.\n");

    return 0;
}

static ssize_t wm8962_ctrl_read(struct file *file,
                        char __user *buf,
                        size_t count,
                        loff_t *pos)
{
    printk("Read data.\n");

    return 0;
}

struct file_operations wm8962_ctrl_fops = {
    .owner      = THIS_MODULE,
    .open       = wm8962_ctrl_open,
    .release    = wm8962_ctrl_close,
    .read       = wm8962_ctrl_read,
};

struct miscdevice wm8962_ctrl_misc = {
    .minor  = MISC_DYNAMIC_MINOR,
    .name   = "wm8962_ctrl_misc",
    .fops   = &wm8962_ctrl_fops,
};

int __init wm8962_ctrl_init(void)
{
    int ret;

	ret = gpio_request(SABRESD_WM8962_POWER, "wm8962_power");
	if ( ret ) {
        printk("get wm8962 power control gpio FAILED!\n");
		return ret;
	}

	gpio_direction_output(SABRESD_WM8962_POWER, 1);

    ret = misc_register(&wm8962_ctrl_misc);
    if(ret)
        printk("wm8962 power control gpio register FAILED!\n");

    printk("wm8962 init over\n");

    return ret;
}


void __exit wm8962_ctrl_exit(void)
{
    misc_deregister(&wm8962_ctrl_misc);

}


module_init(wm8962_ctrl_init);
module_exit(wm8962_ctrl_exit);

MODULE_LICENSE("GPL");

