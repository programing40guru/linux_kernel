#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/cdev.h>
#include <linux/device.h>



dev_t dev=0;
static struct class *dev_class;
static struct cdev pro_cdev;
//mecto make file...


static int      __init driver_start(void);
static void     __exit driver_stop(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *,struct file *);
static ssize_t  device_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  device_write(struct file *filp, const char __user *buf, size_t len, loff_t * off);



static struct file_operations fops={
	 .owner = THIS_MODULE,
	.read=device_read,
	.write=device_write,
	.open=device_open,
	.release=device_release,

};

static int device_open(struct inode *inode, struct file *file)
{
	pr_info("Driver Open Function Called...!!!\n");
	return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
	pr_info("Driver Release Function Called...!!!\n");
	return 0;
}

static ssize_t device_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	pr_info("Driver Read Function Called...!!!\n");
      
	return 0;
}

static ssize_t device_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
	pr_info("Driver Write Function Called...!!!\n");


	return len;
}





static int __init driver_start(void)
{
	/*Allocating Major number*/
	if((alloc_chrdev_region(&dev, 0, 1, "pro_dev")) <0){
		pr_err("Cannot allocate major number\n");
		return -1;
	}
	pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

	//create struct cdev
	cdev_init(&pro_cdev,&fops);
	//adding char device to system

	if((cdev_add(&pro_cdev,dev,1))<0){
		pr_err("Cannot add the device to the system\n");
		goto r_class;

	}

	/*Creating struct class*/
	if(IS_ERR(dev_class = class_create("pro_class"))){
		pr_err("Cannot create the struct class\n");
		goto r_class;
	}
	/*Creating device*/
	if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"pro_char_device"))){
		pr_err("Cannot create the Device 1\n");
		goto r_device;
	}
	pr_info("Device Driver Insert...Done!!!\n");
	return 0;
r_device:
	class_destroy(dev_class);
r_class:
	unregister_chrdev_region(dev,1);
	return -1;
}




static void __exit driver_stop(void)
{
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&pro_cdev);
        unregister_chrdev_region(dev, 1);
        pr_info("Device Driver Remove...Done!!!\n");
}
module_init(driver_start);
module_exit(driver_stop);
MODULE_LICENSE("GPL");

MODULE_VERSION("1.3");


