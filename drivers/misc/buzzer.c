#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/types.h>
#include <linux/io.h>
#include <linux/pwm.h>
#include <linux/fs.h>  

#define BUZZER_ENABLE	182
#define BUZZER_FREQENCY 183
#define BUZZER_DISABLE	184

#define DEV_NAME    "buzzer"

//#define BUZZER_DEBUG
#ifdef BUZZER_DEBUG
	#define debug(fmt, args...) printk(fmt, ##args)
#else
	#define debug(fmt, args...) 
#endif

/*pwm for this buzzer*/
struct pwm_device *pwm = NULL;

static int buzzer_open(struct inode *inode, struct file *filp)
{
    return 0;
}

static int buzzer_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static long buzzer_ioctl(struct file *filp,
                        unsigned int cmd, unsigned long arg)
{
    if(pwm == NULL)
    {
		debug("pwm == NULL.\r\n");
        return -EINVAL;
    }
	if(arg > 20000 || arg < 0)
    {
		debug("out of range.\r\n");
        return -EINVAL;
    }
	debug("cmd: %d,arg: %d,\r\n", cmd, arg);

    switch (cmd) {
	case BUZZER_ENABLE:
        pwm_enable(pwm);
		debug("enable buzzer.\r\n");
		break;
    case BUZZER_FREQENCY:
		if(arg!=0)
		{
        	pwm_config(pwm, 1000000000/arg/2, 1000000000/arg);
			debug("set buzzer pwm freqency.\r\n");
        	break;
		}
	case BUZZER_DISABLE:
        pwm_disable(pwm);
		debug("disable buzzer.\r\n");
		break;
    default:
		debug("Unknown command(%d).\r\n",cmd);
        break;
    }

    return 0;
}

static struct file_operations buzzer_fops = {
    .owner              = THIS_MODULE,
    .unlocked_ioctl     = buzzer_ioctl,
    .open               = buzzer_open,
    .release            = buzzer_release,
};

static struct miscdevice buzzer_miscdev =
{
     .minor  = MISC_DYNAMIC_MINOR,
     .name   = DEV_NAME,
     .fops   = &buzzer_fops,
};

static int __init buzzer_init(void)
{
	/**
     * buzzer?????I.MX6?pwm4?,??????3?(?????)PWM
     */
    pwm = pwm_request(3, "buzzer");
    if ( pwm == NULL ) {
        printk("buzzer request error.\n");
    }

    printk("buzzer_init pass.\n");
    misc_register(&buzzer_miscdev);
    return 0;
}

static void __exit buzzer_exit(void)
{
	pwm_disable(pwm);               // ??pwm
   	pwm_free(pwm);                  // ??pwm
   	pwm = NULL;

   	misc_deregister(&buzzer_miscdev);
}

module_init(buzzer_init);
module_exit(buzzer_exit);

MODULE_DESCRIPTION("pwm_buzzer driver");
MODULE_LICENSE("GPL");
