#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/device.h>

dev_t dev=0;
static struct class *dev_class;


static int __init pro_init(void){

	if((alloc_chrdev_region(&dev, 0, 1, "pro_Dev")) <0){
		pr_err("Cannot allocate major number device\n");
		return -1;
	}

	pr_info("Major= %d Minor=%d\n",MAJOR(dev),MINOR(dev));

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


static void __exit pro_exit(void)
{
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        unregister_chrdev_region(dev, 1);
        pr_info("Kernel Module Removed Successfully...\n");
}

module_init(pro_init);
module_exit(pro_exit);

MODULE_LICENSE("GPL");
MODULE_VERSION("1.2");
