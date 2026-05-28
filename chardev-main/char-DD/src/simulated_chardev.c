#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include "simulated_chardev.h"

#define DEVICE_NAME "simchardev"
#define BUF_SIZE 1024
#define MAX_MSG_LEN 128

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sebastian Ramesh S");
MODULE_DESCRIPTION("Simulated Full Char Device Driver");

static int major;
static char buffer[BUF_SIZE];
static int head = 0;
static int tail = 0;
static int seq_num = 1;
static DEFINE_MUTEX(simc_mutex);

static int device_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "simchardev: Device opened\n");
    return 0;
}

static int device_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "simchardev: Device closed\n");
    return 0;
}

static ssize_t device_read(struct file *filp, char __user *user_buf, size_t len, loff_t *offset) {
    ssize_t ret = 0;
    char msg[MAX_MSG_LEN];

    mutex_lock(&simc_mutex);
    if (head == tail) {
        ret = 0; // buffer empty
    } else {
        snprintf(msg, MAX_MSG_LEN, "[SEQ:%d] %s", seq_num - ((head > tail) ? head - tail : 1), buffer);
        if (copy_to_user(user_buf, msg, strlen(msg) + 1)) {
            ret = -EFAULT;
        } else {
            head = tail;
            ret = strlen(msg);
            printk(KERN_INFO "simchardev: Read message: %s\n", msg);
        }
    }
    mutex_unlock(&simc_mutex);
    return ret;
}

static ssize_t device_write(struct file *filp, const char __user *user_buf, size_t len, loff_t *offset) {
    ssize_t ret = len;
    char msg[MAX_MSG_LEN];

    if (len > MAX_MSG_LEN - 1)
        len = MAX_MSG_LEN - 1;

    if (copy_from_user(msg, user_buf, len))
        return -EFAULT;

    msg[len] = '\0';

    mutex_lock(&simc_mutex);
    if ((tail + len) >= BUF_SIZE) {
        printk(KERN_WARNING "simchardev: Buffer full, dropping message\n");
        ret = -ENOMEM;
    } else {
        snprintf(buffer + tail, len + 1, "%s", msg);
        tail += len;
        printk(KERN_INFO "simchardev: Written message %d: %s\n", seq_num++, msg);
    }
    mutex_unlock(&simc_mutex);
    return ret;
}

static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int status;
    switch (cmd) {
        case SIMC_RESET_BUFFER:
            mutex_lock(&simc_mutex);
            head = 0;
            tail = 0;
            seq_num = 1;
            mutex_unlock(&simc_mutex);
            printk(KERN_INFO "simchardev: Buffer reset via ioctl\n");
            break;
        case SIMC_GET_STATUS:
            mutex_lock(&simc_mutex);
            status = tail - head;
            mutex_unlock(&simc_mutex);
            if (copy_to_user((int __user *)arg, &status, sizeof(int)))
                return -EFAULT;
            printk(KERN_INFO "simchardev: Buffer status ioctl: %d bytes\n", status);
            break;
        default:
            return -EINVAL;
    }
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write,
    .unlocked_ioctl = device_ioctl,
};

static int __init simchardev_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        printk(KERN_ALERT "simchardev: Failed to register device\n");
        return major;
    }
    printk(KERN_INFO "simchardev: Registered with major %d\n", major);
    return 0;
}

static void __exit simchardev_exit(void) {
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "simchardev: Unregistered device\n");
}

module_init(simchardev_init);
module_exit(simchardev_exit);

