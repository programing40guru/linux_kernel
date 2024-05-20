#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include <linux/kthread.h>             //kernel threads
#include <linux/sched.h>               //task_struct 
#include <linux/delay.h>
#include <linux/err.h>


dev_t dev = 0;
static struct class *pro_class;
static struct cdev pro_cdev;
 
static int __init pro_start(void);
static void __exit pro_stop(void);
 
static struct task_struct *pro_thread;
 

static int f_open(struct inode *inode, struct file *file);

static int f_release(struct inode *inode, struct file *file);

static ssize_t f_read(struct file *filp,char __user *buf, size_t len,loff_t * off);
static ssize_t f_write(struct file *filp, const char *buf, size_t len, loff_t * off);
 
int thread_function(void *pv);

int thread_function(void *pv)
{
    int i=0;
    while(!kthread_should_stop()) {
        pr_info("In thead Function %d\n", i++);
        msleep(1000);
    }
    return 0;
}


static struct file_operations fops =
{
	.owner          = THIS_MODULE,
	.read           = f_read,
	.write          = f_write,
	.open           = f_open,
	.release        =f_release,
};

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

	if((alloc_chrdev_region(&dev, 0, 1, "pro_thread")) <0){
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
	if(IS_ERR(device_create(pro_class,NULL,dev,NULL,"pro_thread"))){
		pr_info("Cannot create the Device 1\n");
		goto r_device;
	}

	/*   */

pro_thread = kthread_create(thread_function,NULL,"eTx Thread");
        if(pro_thread) {
            wake_up_process(pro_thread);
        } else {
            pr_err("Cannot create kthread\n");
            goto r_device;
        }
#if 0
        /* You can use this method also to create and run the thread */
        etx_thread = kthread_run(thread_function,NULL,"eTx Thread");
        if(etx_thread) {
            pr_info("Kthread Created Successfully...\n");
        } else {
            pr_err("Cannot create kthread\n");
             goto r_device;
        }
#endif

	pr_info("Device Driver Insert...Done!!!\n");
	return 0;


r_device:
	class_destroy(pro_class);
r_class:
	unregister_chrdev_region(dev,1);
	cdev_del(&pro_cdev);
	return -1;
}

static void __exit pro_stop(void)
{
	kthread_stop(pro_thread);
	device_destroy(pro_class,dev);
	class_destroy(pro_class);
	cdev_del(&pro_cdev);
	unregister_chrdev_region(dev, 1);
	pr_info("Device Driver Remove...Done!!!\n");
}

module_init(pro_start);
module_exit(pro_stop);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Linux device driver (linux kernel)");
MODULE_VERSION("1.9");
