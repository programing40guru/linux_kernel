#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>
#include<linux/fs.h>

//static int __init major_init(void);
//static void __exit major_exit(void);
//creating the dev with custom major and minor number..

dev_t dev = MKDEV(7,2097151);



static int __init major_init(void){

register_chrdev_region(dev,1,"Programm 555"); //numver,counter, name..

printk(KERN_INFO "Major =%d Minor =%d \n",MAJOR(dev),MINOR(dev));
printk(KERN_INFO "module insert sucess..\n");
return 0;

}


static void __exit major_exit(void){
unregister_chrdev_region(dev,1);// number and count

printk(KERN_INFO"exit module from major\n");
}


module_init(major_init);
module_exit(major_exit);

MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
