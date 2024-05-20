#include <linux/module.h>
#include <linux/init.h>
#include <linux/spi/spi.h>
#include <linux/platform_device.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/sysfs.h>

struct kobject *kobj_ref;

static struct task_struct *my_thread;
s16 x,y,z;

u8 read_reg= 0xF2;
u8 data[6];

MODULE_AUTHOR("MEHUL");
MODULE_LICENSE("GPL");

/*************** Sysfs functions **********************/

static ssize_t  sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);

struct kobj_attribute etx_attr = __ATTR(adxl_spi, 0660, sysfs_show, NULL);


static int my_thread_function(void *spi)
{
        while (!kthread_should_stop()) {
                // Perform thread tasks here


                spi_write_then_read(spi,&read_reg,1,data,6);
                //      printk("x0: %d\n",data[0]);

                x=data[1]<<8|data[0];
                y=data[3]<<8|data[2];
                z=data[5]<<8|data[4];

                printk("x: %d\n",x);
                printk("y: %d\n",y);
                printk("z: %d\n",z);
                printk("-----------------\n");

                ssleep(1); // Sleep for 5 seconds
        }
        printk(KERN_INFO "Thread stopping\n");
        return 0;
}

static ssize_t sysfs_show(struct kobject *kobj,struct kobj_attribute *attr, char *buf)
{
        pr_info("Sysfs - Read!!!\n");
        return sprintf(buf, "X data: %d\n Y data: %d\n Z data: %d\n", x,y,z);
}


static int adxl345_probe(struct spi_device *spi)
{

        int ret;
        printk(KERN_INFO "ADXL345 probe function called\n");

        spi->mode = SPI_MODE_3;
        spi->max_speed_hz = 5000000;
        spi->bits_per_word = 8;
        ret = spi_setup(spi);


        u8 config_data[6]= {0x31,0x0B,0x2D,0x08,0x2E,0x80 };
        //configer...
        spi_write(spi,config_data,6);
        // x data


        /*Creating a directory in /sys/kernel/ */
        kobj_ref = kobject_create_and_add("spi_adxl_DATA",NULL);

        /*Creating sysfs file for etx_value*/
        if(sysfs_create_file(kobj_ref,&etx_attr.attr)){
                pr_err("Cannot create sysfs file......\n");

        }


        my_thread = kthread_run(my_thread_function,spi, "my_thread");
        if (IS_ERR(my_thread)) {
                printk(KERN_ERR "Failed to create thread\n");
                return PTR_ERR(my_thread);
        }

        printk(KERN_INFO "Thread created successfully\n");

        return 0;
}

static void  adxl345_remove(struct spi_device *spi)
{
        printk(KERN_INFO "ADXL345 remove function called\n");

        if (my_thread) {
                kthread_stop(my_thread);
                printk(KERN_INFO "Thread stopped\n");
        }
        // Additional cleanup code can be added here
        kobject_put(kobj_ref);
        sysfs_remove_file(kernel_kobj, &etx_attr.attr);



}

static const struct spi_device_id adxl345_id[] = {
        { "adxl345", 0 },
        { }
};
MODULE_DEVICE_TABLE(spi, adxl345_id);

static const struct of_device_id adxl345_of_match[] = {
        { .compatible = "adxl,adxl345" },
        { },
};
MODULE_DEVICE_TABLE(of, adxl345_of_match);

static struct spi_driver adxl345_driver = {
        .driver = {
                .name = "adxl345",
                .owner = THIS_MODULE,
                .of_match_table = adxl345_of_match,
        },
        .probe = adxl345_probe,
        .remove = adxl345_remove,
        .id_table = adxl345_id
};

module_spi_driver(adxl345_driver);

