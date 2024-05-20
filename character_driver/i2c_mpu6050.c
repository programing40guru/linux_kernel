#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include<linux/sysfs.h>
#include<linux/kobject.h>

#define I2C_BUS_AVAILABLE (1)
#define SLAVE_DEVICE_NAME ("MPU6050")
#define MPU6050_SLAVE_ADDR (0x68)
//#define GYRO_SENSITIVITY_SCALE_FACTOR 131.0 // Sensitivity scale factor for ±250 degrees per second

s16 x,y,z;
static struct i2c_adapter *ext_i2c_adapter = NULL;
static struct i2c_client  *ext_i2c_client_mpu6050 = NULL;
static struct task_struct *pro_thread; 
volatile char pro_value[1024];
struct kobject *kobj_ref;

/*************** Sysfs functions **********************/

static ssize_t  sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);

struct kobj_attribute etx_attr = __ATTR(mpu_value, 0660, sysfs_show, NULL);


static int I2C_Write(unsigned char *buf, unsigned int len)
{
	int ret = i2c_master_send(ext_i2c_client_mpu6050, buf, len);

	return ret;
}


static int MPU6050_Init(void) {

	unsigned char buf1[2];
	unsigned char buf2[2];
	unsigned char buf3[2];
	unsigned char buf4[2];
	unsigned char buf5[2];
	//congiger
	buf1[0] = 0x19;
	buf1[1] = 0x07;
	I2C_Write(buf1, 2);
	//power
	//gyro
	//acenolomiter


	buf2[0] = 0x6B;
	buf2[1] = 0x01;
	I2C_Write(buf2, 2);

	buf3[0] = 0x1A;
	buf3[1] = 0x00;
	I2C_Write(buf3, 2);


	buf4[0] = 0x1B;
	buf4[1] = 0x18;
	I2C_Write(buf4, 2);

	//enable 

	buf5[0] = 0x38;
	buf5[1] = 0x01;
	I2C_Write(buf5, 2);

	return 0;
}


static int  thread_function(void *pv)
{

	unsigned char buf[6];
	int rett;
	while(!kthread_should_stop()) {
		// pr_info("In thead Function %d\n", i++);

		// Read gyro data from registers
		rett = i2c_smbus_read_i2c_block_data(ext_i2c_client_mpu6050, 0x43,sizeof(buf), buf);
		if (rett < 0) {
			pr_err("Failed to read gyro data\n");
			// Return rett instead of ret
		}
		// Convert raw data to angular velocity
		x = (s16)((buf[0] << 8) | buf[1]);
		y = (s16)((buf[2] << 8) | buf[3]);
		z = (s16)((buf[4] << 8) | buf[5]);
		pr_info("Gyro X: %d\n Gyro Y: %d\n Gyro Y: %d\n");


		msleep(1000);
	}
	return 0;
}
static ssize_t sysfs_show(struct kobject *kobj,struct kobj_attribute *attr, char *buf)
{
	pr_info("Sysfs - Read!!!\n");
	return sprintf(buf, "X data: %d\n Y data: %d\n Z data: %d\n", x,y,z);
}


static int etx_mpu6050_probe(struct i2c_client *client) {

	u8 id_reg;
	int ret = MPU6050_Init();
	if (ret == 0) {
		pr_info("MPU6050 Probed!!!\n");
	} else {
		pr_err("Failed to probe MPU6050!!!\n");
	}
	//ext_i2c_client_mpu6050 = client;


	// Read WHO_AM_I register
	ret = i2c_smbus_read_byte_data(client, 0x75); // WHO_AM_I register address
	if (ret < 0) {
		printk(KERN_ERR "Failed to read MPU6050 ID\n");
		return ret;
	}

	id_reg = ret;
	printk(KERN_INFO "MPU6050 ID: 0x%02X\n", id_reg);




	printk(KERN_INFO"okay done");
	return 0;
}




static void etx_mpu6050_remove(struct i2c_client *client) {
	pr_info("MPU6050 Removed!!!\n");
	if (pro_thread) {
		kthread_stop(pro_thread);
		pro_thread = NULL;
	}

}

static const struct i2c_device_id etx_mpu6050_id[] = {
	{ SLAVE_DEVICE_NAME, 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, etx_mpu6050_id);

static struct i2c_driver etx_mpu6050_driver = {
	.driver = {
		.name   = SLAVE_DEVICE_NAME,
		.owner  = THIS_MODULE,
	},
	.probe          = etx_mpu6050_probe,
	.remove         = etx_mpu6050_remove,
	.id_table       = etx_mpu6050_id,
};

static struct i2c_board_info mpu6050_i2c_board_info = {
	I2C_BOARD_INFO(SLAVE_DEVICE_NAME, MPU6050_SLAVE_ADDR)
};

static int __init etx_driver_init(void) {
	int ret = -1;

	ext_i2c_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);

	if (ext_i2c_adapter != NULL) {
		ext_i2c_client_mpu6050 = i2c_new_client_device(ext_i2c_adapter, &mpu6050_i2c_board_info);
		if (ext_i2c_client_mpu6050 != NULL) {
			i2c_add_driver(&etx_mpu6050_driver);
			ret = 0;
		}

		i2c_put_adapter(ext_i2c_adapter);
	}
	/*Creating a directory in /sys/kernel/ */
	kobj_ref = kobject_create_and_add("MPU6050_DATA",NULL);

	/*Creating sysfs file for etx_value*/
	if(sysfs_create_file(kobj_ref,&etx_attr.attr)){
		pr_err("Cannot create sysfs file......\n");

	}



	pro_thread = kthread_create(thread_function,NULL,"eTx Thread");
	if(pro_thread) {
		wake_up_process(pro_thread);
	} else {
		pr_err("Cannot create kthread\n");

	}


	pr_info("Driver Added!!!\n");

	return ret;
}

static void __exit etx_driver_exit(void) {

	kobject_put(kobj_ref);
	sysfs_remove_file(kernel_kobj, &etx_attr.attr);

	i2c_unregister_device(ext_i2c_client_mpu6050);
	i2c_del_driver(&etx_mpu6050_driver);
	pr_info("Driver Removed!!!\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("pro40guru");

