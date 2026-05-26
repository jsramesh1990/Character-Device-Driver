
# Linux Character Device Driver

## 1. Introduction

A **Character Device Driver** in Linux is a driver that transfers data between user space and hardware **one character (byte) at a time**.

Character drivers are the most common and beginner-friendly Linux drivers.  
They provide a simple stream-based interface using standard system calls like:

- open()
- read()
- write()
- close()
- ioctl()

Examples of character devices:

- UART / Serial ports
- Keyboards
- GPIO devices
- Sensors
- RTC devices
- ADC devices
- LED drivers

Character drivers usually create device files inside:

```bash
/dev/
````

Example:

```bash
/dev/ttyS0
/dev/i2c-1
/dev/mydevice
```

---

# 2. Why Do We Use Character Drivers?

Without a character driver, user applications cannot directly communicate with hardware safely.

Character drivers provide:

* Hardware abstraction
* Standardized interface
* Secure access
* Controlled communication
* Kernel-managed resource sharing

Applications use normal Linux APIs:

```c
open()
read()
write()
ioctl()
```

instead of directly touching hardware registers.

---

# 3. Real-Time Examples

| Device                 | Type           | Why Character Driver      |
| ---------------------- | -------------- | ------------------------- |
| UART / Serial Port     | Byte stream    | Data arrives byte-by-byte |
| Keyboard               | Input stream   | Sends characters/events   |
| Temperature Sensor     | Sensor data    | Reads small chunks        |
| RTC (Real-Time Clock)  | Time device    | Read/write operations     |
| GPIO LED Driver        | Control device | ON/OFF commands           |
| I2C EEPROM             | Memory access  | Sequential byte transfer  |
| Touchscreen Controller | Input events   | Event stream              |

---

# 4. Character Driver Architecture

```text
+---------------------------+
| User Space Application    |
|---------------------------|
| open()                    |
| read()                    |
| write()                   |
| ioctl()                   |
+-------------+-------------+
              |
              v
+---------------------------+
| Device File (/dev/mydev)  |
+-------------+-------------+
              |
              v
+---------------------------+
| Character Driver          |
|---------------------------|
| file_operations           |
| open()                    |
| read()                    |
| write()                   |
| release()                 |
+-------------+-------------+
              |
              v
+---------------------------+
| Physical Hardware         |
+---------------------------+
```

---

# 5. Important Terminology

## Major Number

Identifies the driver.

Example:

```bash
240
```

## Minor Number

Identifies the device instance handled by the same driver.

Example:

```bash
/ dev/mydev0
/ dev/mydev1
```

---

# 6. Core Components of Character Driver

## Required Header Files

```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
```

---

# 7. Basic Driver Flow

## Step 1 – Register Driver

```c
alloc_chrdev_region()
```

Allocates major/minor numbers.

---

## Step 2 – Initialize cdev Structure

```c
cdev_init()
```

Connects file operations with the kernel.

---

## Step 3 – Add Driver to Kernel

```c
cdev_add()
```

Registers driver with VFS.

---

## Step 4 – Create Device Class

```c
class_create()
```

Used for automatic `/dev` node creation.

---

## Step 5 – Create Device File

```c
device_create()
```

Creates:

```bash
/dev/mydevice
```

---

# 8. Full Character Driver Example

## char_driver.c

```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>

#define DEVICE_NAME "mychardev"
#define BUFFER_SIZE 1024

static dev_t dev_num;
static struct cdev my_cdev;
static struct class *my_class;

static char kernel_buffer[BUFFER_SIZE];

/* OPEN */
static int my_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Device Opened\n");
    return 0;
}

/* READ */
static ssize_t my_read(struct file *file,
                       char __user *buf,
                       size_t len,
                       loff_t *offset)
{
    copy_to_user(buf, kernel_buffer, len);

    printk(KERN_INFO "Data Read\n");

    return len;
}

/* WRITE */
static ssize_t my_write(struct file *file,
                        const char __user *buf,
                        size_t len,
                        loff_t *offset)
{
    copy_from_user(kernel_buffer, buf, len);

    printk(KERN_INFO "Data Written\n");

    return len;
}

/* CLOSE */
static int my_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Device Closed\n");
    return 0;
}

/* File Operations */
static struct file_operations fops =
{
    .owner = THIS_MODULE,
    .open = my_open,
    .read = my_read,
    .write = my_write,
    .release = my_release,
};

/* INIT FUNCTION */
static int __init char_driver_init(void)
{
    alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);

    cdev_init(&my_cdev, &fops);

    cdev_add(&my_cdev, dev_num, 1);

    my_class = class_create(THIS_MODULE, "my_class");

    device_create(my_class, NULL, dev_num, NULL, DEVICE_NAME);

    printk(KERN_INFO "Character Driver Loaded\n");

    return 0;
}

/* EXIT FUNCTION */
static void __exit char_driver_exit(void)
{
    device_destroy(my_class, dev_num);

    class_destroy(my_class);

    cdev_del(&my_cdev);

    unregister_chrdev_region(dev_num, 1);

    printk(KERN_INFO "Character Driver Unloaded\n");
}

module_init(char_driver_init);
module_exit(char_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple Linux Character Driver");
```

---

# 9. File Operations Explained

## open()

Called when application opens device.

Example:

```c
open("/dev/mychardev", O_RDWR);
```

Purpose:

* Initialize device
* Allocate resources
* Check permissions

---

## read()

Transfers data from kernel space to user space.

Uses:

```c
copy_to_user()
```

---

## write()

Transfers data from user space to kernel space.

Uses:

```c
copy_from_user()
```

---

## release()

Called when device is closed.

Purpose:

* Free resources
* Stop hardware
* Cleanup

---

# 10. Makefile

```Makefile
obj-m += char_driver.o

KDIR = /lib/modules/$(shell uname -r)/build
PWD  = $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
```

---

# 11. Compile the Driver

```bash
make
```

Output:

```bash
char_driver.ko
```

---

# 12. Load the Driver

```bash
sudo insmod char_driver.ko
```

Check:

```bash
lsmod | grep char_driver
```

---

# 13. Verify Device File

```bash
ls /dev/mychardev
```

---

# 14. View Kernel Logs

```bash
dmesg | tail
```

---

# 15. Test Driver from User Space

## test_app.c

```c
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    int fd;
    char write_buf[] = "Hello Driver";
    char read_buf[100];

    fd = open("/dev/mychardev", O_RDWR);

    write(fd, write_buf, sizeof(write_buf));

    read(fd, read_buf, sizeof(read_buf));

    printf("Read Data: %s\n", read_buf);

    close(fd);

    return 0;
}
```

Compile:

```bash
gcc test_app.c -o test_app
```

Run:

```bash
./test_app
```

---

# 16. Advantages of Character Drivers

| Advantage        | Description                   |
| ---------------- | ----------------------------- |
| Simple Design    | Easy to learn and implement   |
| Stream Interface | Good for serial communication |
| Flexible         | Supports many hardware types  |
| Standard APIs    | Uses Linux system calls       |
| Lightweight      | Low overhead                  |
| Modular          | Can load/unload dynamically   |

---

# 17. Disadvantages of Character Drivers

| Disadvantage               | Description                   |
| -------------------------- | ----------------------------- |
| Slow for Large Data        | Byte-by-byte transfer         |
| No Random Access           | Unlike block devices          |
| Synchronization Complexity | Multi-process access issues   |
| Kernel Crash Risk          | Bugs can crash kernel         |
| Security Risks             | Improper validation dangerous |

---

# 18. Character Driver vs Block Driver

| Feature       | Character Driver | Block Driver    |
| ------------- | ---------------- | --------------- |
| Data Access   | Byte stream      | Block-based     |
| Random Access | No               | Yes             |
| Buffering     | Minimal          | Heavy buffering |
| Example       | UART             | HDD/SSD         |
| Device File   | /dev/ttyS0       | /dev/sda        |

---

# 19. Important Kernel APIs

| API                 | Purpose                     |
| ------------------- | --------------------------- |
| alloc_chrdev_region | Allocate device numbers     |
| cdev_init           | Initialize character device |
| cdev_add            | Register device             |
| class_create        | Create sysfs class          |
| device_create       | Create device node          |
| copy_to_user        | Kernel → User               |
| copy_from_user      | User → Kernel               |

---

# 20. Common Interview Questions

## Q1. What is a Character Driver?

A Linux driver that transfers data as a stream of bytes between hardware and user space.

---

## Q2. Difference Between Character and Block Driver?

Character drivers process byte streams.
Block drivers process fixed-size blocks.

---

## Q3. Why Use copy_to_user()?

Kernel memory cannot be directly accessed by user space.
copy_to_user safely transfers data.

---

## Q4. What is file_operations?

A structure containing pointers to driver callback functions.

Example:

```c
.open
.read
.write
.release
.ioctl
```

---

## Q5. What Happens When User Calls read()?

Flow:

```text
User Application
    ↓
VFS Layer
    ↓
Driver read()
    ↓
Hardware Access
    ↓
copy_to_user()
```

---

# 21. Common Errors

## Error: Device Busy

Cause:

* Device already opened

Fix:

* Use mutex/spinlock

---

## Error: Invalid Module Format

Cause:

* Kernel version mismatch

Fix:

```bash
uname -r
```

Rebuild using correct headers.

---

## Error: Segmentation Fault

Cause:

* Invalid user pointer

Fix:

* Validate user buffers
* Use copy_to_user safely

---

# 22. Advanced Topics

After learning basic character drivers, move to:

* ioctl()
* poll/select
* wait queues
* interrupts
* GPIO drivers
* I2C drivers
* SPI drivers
* platform drivers
* Device Tree
* DMA
* mmap()

---

# 23. Best Practices

## Use Dynamic Allocation

Prefer:

```c
alloc_chrdev_region()
```

instead of hardcoded major numbers.

---

## Always Check Return Values

```c
if (ret < 0)
    return ret;
```

---

## Proper Cleanup

Every allocation must be freed in exit().

---

## Protect Shared Resources

Use:

* mutex
* spinlock
* atomic variables

---
