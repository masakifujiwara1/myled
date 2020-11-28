// SPDX-License-Identifier: GPL-3.0
// (c) 2020 RyuichiUeda and MasakiFujiwara
/*
 * driver for LED control
 */

#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/uaccess.h>
#include<linux/io.h>
#include<linux/delay.h>

MODULE_AUTHOR("Ryuichi Ueda and Masaki Fujiwara");
MODULE_DESCRIPTION("driver for LED control");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.0.1");

static dev_t dev;
static struct cdev cdv;
static struct class *cls = NULL;
static volatile u32 *gpio_base = NULL;

static ssize_t led_write(struct file* filp, const char* buf, size_t count, loff_t* pos)
{
	char c;
	if(copy_from_user(&c, buf, sizeof(char)))
		return -EFAULT;

//	printk(KERN_INFO "receive %c\n", c);
	if(c == '0')
	{
		gpio_base[10] = 1 << 25;
	}else if(c == '1')
	{
		gpio_base[7] = 1 << 25;
	}else if(c == '2')
	{
		int count, s, o;
		int onetime=100;//0.1s

		for(count=0;count<3;count++)
		{
			for(s=0;s<3;s++)//sosのsの・・・部分
			{
				gpio_base[7] = 1 << 25;
				mdelay(onetime);//ms遅らせる
				gpio_base[10] = 1 << 25;
				mdelay(onetime);
			}
			mdelay(2*onetime);
			for(o=0;o<3;o++)//sosのoの---部分
			{
				gpio_base[7] = 1 << 25;     
				mdelay(3*onetime);
				gpio_base[10] = 1 << 25;
				mdelay(onetime);
			}
			mdelay(2*onetime);
			for(s=0;s<3;s++)
			{
				gpio_base[7] = 1 <<25;
				mdelay(onetime);
				gpio_base[10] = 1 <<25;
				mdelay(onetime);
			}
				mdelay(10*onetime);

		}

	}else if(c == '3')
	{
		int j, n, stage=200;//stage=ms/dutycycle
		int ms=2000000;//2000ms
		int dutycycle=10000;//10ms 実際は周期
		int dutyratio=200;//0.2ms 実際はdutycycle (dutyratio(ms)/dutycycle(ms))*100=2%
		int ontime, offtime, amount;//amount:duty比の変化量

		ms/=dutycycle;
		amount=(dutycycle-dutyratio)/stage;

		//ontime=dutyratio*dutycyle;
		//offtime=dutycyle-ontime;

		for(j=0;j<3;j++)
		{
			for(n=0;n<ms;n++)//2sかけて0%→100%
			{
				//dutyratio+=amount;
				ontime=dutyratio;
				offtime=dutycycle-ontime;
				dutyratio+=amount;
				gpio_base[7] = 1 << 25;
				udelay(ontime);//us遅らせる
				gpio_base[10] = 1 << 25;
				udelay(offtime);
			}
			for(n=0;n<ms;n++)//2sかけて100%→0%
			{
				ontime=dutyratio;
				offtime=dutycycle-ontime;
				dutyratio-=amount;
				gpio_base[7] = 1 << 25;
				udelay(ontime);
				gpio_base[10] = 1 << 25;
				udelay(offtime);
			}
			mdelay(1000);
		}
		
	}




	return 1;
}
static ssize_t sushi_read(struct file* filp, char* buf, size_t count, loff_t* pos)
{
	int size = 0;
	char sushi[] = { 's', 'u', 's', 'h', 'i', 0x0A};
	if(copy_to_user(buf+size, (const char *)sushi, sizeof(sushi)))
	{
		printk(KERN_ERR "sushi: copy_to_user failed \n");
		return -EFAULT;
	}

	size += sizeof(sushi);
	return size;

}

static struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.write = led_write,
	.read = sushi_read
};

static int __init init_mod(void)
{
	int retval;
	retval = alloc_chrdev_region(&dev, 0, 1, "myled");
	if (retval < 0) 
	{
		printk(KERN_ERR "alloc_chrdev_region failed \n");
		return retval;
	}
	printk(KERN_INFO "%s is loaded. major:%d\n", __FILE__, MAJOR(dev));

	cdev_init(&cdv, &led_fops);
	retval = cdev_add(&cdv, dev, 1);
	if(retval < 0)
	{
		printk(KERN_ERR "cdev_add failed. major:%d, minor:%d\n", MAJOR(dev), MINOR(dev));
		return retval;
	}

	cls = class_create(THIS_MODULE, "myled");
	if(IS_ERR(cls))
	{
		printk(KERN_ERR "class_create failed\n");
		return PTR_ERR(cls);
	}
	device_create(cls, NULL, dev, NULL, "myled%d", MINOR(dev));

	gpio_base = ioremap_nocache(0x3f200000, 0xA0);

	const u32 led = 25;
	const u32 index = led/10;
	const u32 shift = (led%10)*3;
	const u32 mask = ~(0x7<<shift);
	gpio_base[index] = (gpio_base[index] & mask) | (0x1 << shift);

	return 0;
}

static void __exit cleanup_mod(void)
{
	cdev_del(&cdv);
	device_destroy(cls, dev);
	class_destroy(cls);
	unregister_chrdev_region(dev, 1);
	printk(KERN_INFO "%s is unloaded. major:%d\n", __FILE__, MAJOR(dev));
}


module_init(init_mod);
module_exit(cleanup_mod);

