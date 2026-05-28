#ifndef SIMULATED_CHARDEV_H
#define SIMULATED_CHARDEV_H

#include <linux/ioctl.h>

#define SIMC_IOC_MAGIC 'k'
#define SIMC_RESET_BUFFER _IO(SIMC_IOC_MAGIC, 0)
#define SIMC_GET_STATUS   _IOR(SIMC_IOC_MAGIC, 1, int)

#endif // SIMULATED_CHARDEV_H

