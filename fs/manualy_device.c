//sudo mknod -m 666 /dev/pro_neww   c 246 0
//new node file dives..


#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>

dev_t dev = 0;

static int __init major_init(void)
{

        int a = alloc_chrdev_region(&dev, 0, 1, "programming_create dev");
        if(a <0){
                printk(KERN_INFO "can not allocate major number\n");
                return -1;
        }
        printk(KERN_INFO "kernel modual creted sucess %d",a);

        return 0;
}

static void __exit major_exit(void)
{
        unregister_chrdev_region(dev, 1);
        printk(KERN_INFO "Kernel Module Removed Successfully...\n");
}

module_init(major_init);
module_exit(major_exit);

MODULE_LICENSE("GPL");
