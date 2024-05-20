#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include<linux/sysfs.h> 
#include<linux/kobject.h> 
#include <linux/err.h>
volatile char pro_value[1024];

//volatile int pro_value=0;

dev_t dev=0;
static struct class *pro_class;  //calss
static struct cdev pro_cdev;     // cdev

struct kobject *kobj_ref;

static int      __init pro_start(void);
static void     __exit pro_stop(void);

/*************** Driver functions **********************/
static int      f_open(struct inode *inode, struct file *file);

static int      f_release(struct inode *inode, struct file *file);

static ssize_t  f_read(struct file *filp, char __user *buf, size_t len,loff_t * off);

static ssize_t  f_write(struct file *filp,const char *buf, size_t len, loff_t * off);


/*************** Sysfs functions **********************/

static ssize_t  sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);

static ssize_t  sysfs_store(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count);

struct kobj_attribute etx_attr = __ATTR(pro_value, 0660, sysfs_show, sysfs_store);


static struct file_operations fops =
{
	.owner          = THIS_MODULE,
	.read           = f_read,
	.write          = f_write,
	.open           = f_open,
	.release        =f_release,
};


// show t read data 



static ssize_t sysfs_show(struct kobject *kobj,struct kobj_attribute *attr, char *buf)
{
	pr_info("Sysfs - Read!!!\n");
	return sprintf(buf, "%s", pro_value);
}


// store to  write....


static ssize_t sysfs_store(struct kobject *kobj, 
		struct kobj_attribute *attr,const char *buf, size_t count)
{
	pr_info("Sysfs - Write!!!\n");
	sscanf(buf,"%s",pro_value);
	return count;
}

//open device file

static int f_open(struct inode *inode, struct file *file)
{
	pr_info("Device File Opened...!!!\n");
	return 0;
}

//close device file

static int f_release(struct inode *inode, struct file *file)
{
	pr_info("Device File Closed...!!!\n");
	return 0;
}


//fread file.. 

static ssize_t f_read(struct file *filp,char __user *buf, size_t len, loff_t *off)
{
	pr_info("Read function\n");
	return 0;
}

//write file..

static ssize_t f_write(struct file *filp,const char __user *buf, size_t len, loff_t *off)
{
	pr_info("Write Function\n");
	return len;
}


static int __init pro_start(void)
{
	//allocate major number. 

	if((alloc_chrdev_region(&dev, 0, 1, "pro_sfs_dev")) <0){
		pr_info("Cannot allocate major number\n");
		return -1;
	}
	pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));


	/*Creating cdev structure*/
	cdev_init(&pro_cdev,&fops);


	/*Adding character device to the system*/
	if((cdev_add(&pro_cdev,dev,1)) < 0){
		pr_info("Cannot add the device to the system\n");
		goto r_class;
	}

	/*Creating struct class*/
	if(IS_ERR(pro_class = class_create("pro_class"))){
		pr_info("Cannot create the struct class\n");
		goto r_class;
	}

	/*Creating device*/
	if(IS_ERR(device_create(pro_class,NULL,dev,NULL,"pro_sfs_dev"))){
		pr_info("Cannot create the Device 1\n");
		goto r_device;
	}

	/*Creating a directory in /sys/kernel/ */
	kobj_ref = kobject_create_and_add("pro_sfs_file",kernel_kobj);

	/*Creating sysfs file for etx_value*/
	if(sysfs_create_file(kobj_ref,&etx_attr.attr)){
		pr_err("Cannot create sysfs file......\n");
		goto r_sysfs;
	}


	pr_info("Device Driver Insert...Done!!!\n");
	return 0;

r_sysfs:
	kobject_put(kobj_ref); 
	sysfs_remove_file(kernel_kobj, &etx_attr.attr);

r_device:
	class_destroy(pro_class);
r_class:
	unregister_chrdev_region(dev,1);
	cdev_del(&pro_cdev);
	return -1;
}


static void __exit pro_stop(void)
{
	kobject_put(kobj_ref); 
	sysfs_remove_file(kernel_kobj, &etx_attr.attr);
	device_destroy(pro_class,dev);
	class_destroy(pro_class);
	cdev_del(&pro_cdev);
	unregister_chrdev_region(dev, 1);
	pr_info("Device Driver Remove...Done!!!\n");
}

module_init(pro_start);
module_exit(pro_stop);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Linux device driver (sysfs)");
MODULE_VERSION("1.8");
