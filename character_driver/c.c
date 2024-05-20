#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define MPU6050_ADDR 0x68 // Default I2C address of MPU6050

static struct i2c_client *mpu6050_client;

static int mpu6050_read_data(int reg, int len, u8 *data) {
    return i2c_smbus_read_i2c_block_data(mpu6050_client, reg, len, data);
}

static ssize_t mpu6050_read(struct file *file, char *buf, size_t count, loff_t *ppos) {
    u8 data[14];
    int err;

    err = mpu6050_read_data(0x3B, sizeof(data), data);
    if (err < 0)
        return err;

//    if (copy_to_user(buf, data, sizeof(data)))
//        return -EFAULT;
prink(KERN_INFO"readded data ");
    return sizeof(data);
}

static const struct file_operations mpu6050_fops = {
    .owner = THIS_MODULE,
    .read = mpu6050_read,
};

static int mpu6050_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    int err;

    mpu6050_client = client;

    // Initialization code for MPU6050 sensor
    // For example, configure the sensor for desired settings
    // For simplicity, let's assume the sensor is already configured

    // Register file operations
    if ((err = register_chrdev(0, "mpu6050", &mpu6050_fops)) < 0) {
        dev_err(&client->dev, "failed to register character device\n");
        return err;
    }

    return 0;
}

static int mpu6050_remove(struct i2c_client *client) {
    unregister_chrdev(0, "mpu6050");
    return 0;
}

static const struct of_device_id mpu6050_of_match[] = {
    { .compatible = "invensense,mpu6050" },
    { }
};
MODULE_DEVICE_TABLE(of, mpu6050_of_match);

static struct i2c_driver mpu6050_driver = {
    .driver = {
        .name = "mpu6050",
        .of_match_table = mpu6050_of_match,
    },
    .probe = mpu6050_probe,
    .remove = mpu6050_remove,
};

static int __init mpu6050_init(void) {
    return i2c_add_driver(&mpu6050_driver);
}

static void __exit mpu6050_exit(void) {
    i2c_del_driver(&mpu6050_driver);
}

module_init(mpu6050_init);
module_exit(mpu6050_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("MPU6050 I2C Driver");

