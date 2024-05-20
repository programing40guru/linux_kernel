#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>                 //kmalloc()
#include <linux/uaccess.h>              //copy_to/from_user()
#include <linux/err.h>


#define BUF_SIZE  1024 

dev_t dev = 0;
static struct class *dev_class;
static struct cdev pro_cdev;
uint8_t *buffer;

static int     __init pro_start(void);
static void     __exit pro_stop(void);
static int      f_open(struct inode *inode, struct file *file);
static int      f_release(struct inode *inode, struct file *file);
static ssize_t  f_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  f_write(struct file *filp, const char __user *buf, size_t len, loff_t * off);

static struct file_operations fops =
{
	.owner          = THIS_MODULE,
	.read           = f_read,
	.write          = f_write,
	.open           = f_open,
	.release        = f_release,
};


static int f_open(struct inode *inode, struct file *file)
{
//	kmalloc(*buffer)
//	kmalloc(*buffer)

	buffer = kmalloc(BUF_SIZE , GFP_KERNEL);
	pr_info("Driver Open Function Called...!!!\n");
	return 0;
}


static int f_release(struct inode *inode, struct file *file)
{
	kfree(buffer);
	pr_info("Device File Closed...!!!\n");
	return 0;
}

static ssize_t f_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	//Copy the data from the kernel space to the user-space
	if(copy_to_user(buf, buffer, BUF_SIZE) )
	{
		pr_err("Data Read : Err!\n");
	}
	pr_info("Data Read : Done!\n");
	return len;
}



static ssize_t f_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
	//Copy the data to kernel space from the user-space
	if( copy_from_user(buffer, buf, len) )
	{
		pr_err("Data Write : Err!\n");
	}
	pr_info("Data Write : Done!\n");
	return len;
}

static int __init pro_start(void){

	if((alloc_chrdev_region(&dev, 0, 1, "pro_dev")) <0){
		pr_err("Cannot allocate major number device\n");
		return -1;
	}

	pr_info("Major= %d Minor=%d\n",MAJOR(dev),MINOR(dev));


cdev_init(&pro_cdev,&fops);
 
        /*Adding character device to the system*/
        if((cdev_add(&pro_cdev,dev,1)) < 0){
            pr_info("Cannot add the device to the system\n");
            goto r_class;
        }

	//	dev_class = class_create(NULL, "pro_class");
	dev_class= class_create("pro_class");
	if(IS_ERR(dev_class)){
		pr_err("can not create the struct class for device\n");
		goto r_class;
	}

	if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"pro_dev"))){

		pr_err("Cannot create the Device\n");
		goto r_device;
	}
	
	pr_info("kernel moduleinsert sucess...\n");

	return 0;

r_device :
	class_destroy(dev_class);

r_class :
	unregister_chrdev_region(dev,1);
	return -1;
}

static void __exit pro_stop(void)
{
  kfree(buffer);
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&pro_cdev);
        unregister_chrdev_region(dev, 1);
        pr_info("Device Driver Remove...Done!!!\n");
}


module_init(pro_start);
module_exit(pro_stop);

MODULE_LICENSE("GPL");
MODULE_VERSION("1.2.1");

