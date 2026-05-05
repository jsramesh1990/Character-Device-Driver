# Simple Character Device Driver

![Linux](https://img.shields.io/badge/Linux-Kernel_Module-blue.svg)
![C](https://img.shields.io/badge/Language-C-blue.svg)
![Yocto](https://img.shields.io/badge/Yocto-Project-brightgreen.svg)
![License](https://img.shields.io/badge/License-GPLv3-green.svg)
![Kernel](https://img.shields.io/badge/Kernel-5.x-orange.svg)
![Build](https://img.shields.io/badge/Build-Make-success.svg)

A comprehensive demonstration of Linux character device driver development showcasing communication between **User-space** and **Kernel-space** through a custom character device.

## Table of Contents
- [Project Flow](#project-flow)
- [Architecture](#architecture)
- [Working Flow](#working-flow)
- [Yocto Integration](#yocto-integration)
- [Prerequisites](#prerequisites)
- [Quick Start](#quick-start)
- [Procedure](#procedure)
- [Testing](#testing)
- [Troubleshooting](#troubleshooting)

## Project Flow

```
+----------------+     open()     +------------------+
|   User App     | -------------> |   /dev/mychardev  |
+----------------+                +------------------+
        ^                                 |
        |                                 | VFS Layer
        |                                 v
        |                        +------------------+
        |                        | Character Device |
        |                        |     Driver       |
        |                        +------------------+
        |                                 |
        |                                 | Kernel Space
        |                                 v
        |                        +------------------+
        +------------------------+   copy_to_user   |
                                 |   Kernel Buffer  |
                                 +------------------+
```

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                        USER SPACE                            │
│  ┌──────────────────┐              ┌──────────────────┐    │
│  │  User Application│              │  /dev/mychardev  │    │
│  │   (./user_app)   │──────────────│   (Device Node)  │    │
│  └──────────────────┘              └──────────────────┘    │
└─────────────────────────┬───────────────────────────────────┘
                          │ System Calls
                          │ open/read/write/close
┌─────────────────────────▼───────────────────────────────────┐
│                        KERNEL SPACE                          │
│  ┌──────────────────────────────────────────────────────┐   │
│  │              Virtual File System (VFS)               │   │
│  └─────────────────────────┬────────────────────────────┘   │
│                            │                                 │
│  ┌─────────────────────────▼────────────────────────────┐   │
│  │      Character Device Driver (mychardev.ko)          │   │
│  │  ┌────────────────────────────────────────────────┐  │   │
│  │  │         File Operations Structure              │  │   │
│  │  │  .open = my_open                               │  │   │
│  │  │  .read = my_read                               │  │   │
│  │  │  .write = my_write                             │  │   │
│  │  │  .release = my_release                         │  │   │
│  │  └────────────────────────────────────────────────┘  │   │
│  │                                                       │   │
│  │  ┌────────────────────────────────────────────────┐  │   │
│  │  │           Kernel Buffer (mydata[])             │  │   │
│  │  └────────────────────────────────────────────────┘  │   │
│  └───────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

## Working Flow

```
Driver Initialization:
======================
    module_init()
         │
         ▼
    alloc_chrdev_region()  ──────► Gets Major Number (e.g., 240)
         │
         ▼
    cdev_init() & cdev_add()
         │
         ▼
    Driver Registered in Kernel

Device Operation:
================
    User App                    VFS                     Device Driver
       │                         │                           │
       │   open("/dev/mychardev")│                           │
       │────────────────────────►│                           │
       │                         │      my_open()           │
       │                         │──────────────────────────►│
       │                         │                           │
       │                         │      Success              │
       │                         │◄──────────────────────────│
       │◄───File Descriptor──────│                           │
       │                         │                           │
       │   write(fd, "Hello", 5) │                           │
       │────────────────────────►│                           │
       │                         │      my_write()           │
       │                         │──────────────────────────►│
       │                         │                           │
       │                         │    copy_from_user()       │
       │                         │      [Store in buffer]    │
       │                         │                           │
       │                         │      Bytes Written (5)    │
       │                         │◄──────────────────────────│
       │◄───5 bytes written──────│                           │
       │                         │                           │
       │   read(fd, buffer, 5)   │                           │
       │────────────────────────►│                           │
       │                         │      my_read()            │
       │                         │──────────────────────────►│
       │                         │                           │
       │                         │     copy_to_user()        │
       │                         │   [Read from buffer]      │
       │                         │                           │
       │                         │      Bytes Read (5)       │
       │                         │◄──────────────────────────│
       │◄───"Hello" (5 bytes)────│                           │
       │                         │                           │
       │   close(fd)             │                           │
       │────────────────────────►│                           │
       │                         │      my_release()         │
       │                         │──────────────────────────►│
       │                         │                           │
       │                         │      Cleanup Complete     │
       │                         │◄──────────────────────────│
       │◄───Device Closed────────│                           │

Driver Cleanup:
===============
    module_exit()
         │
         ▼
    cdev_del()
         │
         ▼
    unregister_chrdev_region()
         │
         ▼
    Driver Removed
```

## Yocto Integration

### Yocto Build Flow

```
┌─────────────────────────────────────────────────────────────────┐
│                     YOCTO BUILD ENVIRONMENT                      │
│                                                                   │
│  ┌──────────────┐     ┌──────────────┐     ┌──────────────┐    │
│  │ Yocto Setup  │────►│ Create Layer │────►│Driver Recipe │    │
│  └──────────────┘     └──────────────┘     └──────────────┘    │
│                                                    │              │
│                                                    ▼              │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │                    Layer Structure                        │   │
│  │  meta-mychardev/                                          │   │
│  │  ├── conf/                                                │   │
│  │  │   └── layer.conf                                       │   │
│  │  ├── recipes-kernel/                                      │   │
│  │  │   ├── linux/                                           │   │
│  │  │   │   └── linux-yocto_%.bbappend                      │   │
│  │  │   └── mychardev/                                       │   │
│  │  │       ├── mychardev.bb                                 │   │
│  │  │       ├── mychardev.c                                  │   │
│  │  │       └── Makefile                                     │   │
│  │  └── recipes-example/                                     │   │
│  │      └── mychardev-init/                                  │   │
│  │          └── mychardev-init.bb                            │   │
│  └──────────────────────────────────────────────────────────┘   │
│                              │                                    │
│                              ▼                                    │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │                   Build Process                           │   │
│  │  bitbake core-image-minimal                               │   │
│  │       │                                                    │   │
│  │       ▼                                                    │   │
│  │  Kernel Built with mychardev.ko                           │   │
│  │       │                                                    │   │
│  │       ▼                                                    │   │
│  │  Root Filesystem Created                                   │   │
│  │       │                                                    │   │
│  │       ▼                                                    │   │
│  │  Device Node Rules Added                                   │   │
│  └──────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                      TARGET DEVICE                               │
│                                                                   │
│  1. Deploy Image to Target                                       │
│  2. Boot Target                                                  │
│  3. Verify: lsmod | grep mychardev                              │
│  4. Test: echo "Hello" > /dev/mychardev                         │
│  5. Read: cat /dev/mychardev                                     │
└─────────────────────────────────────────────────────────────────┘
```

### Yocto Integration Procedure

**Step 1: Create Custom Layer**
```bash
mkdir -p yocto/meta-mychardev/{recipes-kernel,recipes-example,conf}
cd yocto/meta-mychardev
```

**Step 2: Layer Configuration (conf/layer.conf)**
```bitbake
BBPATH .= ":${LAYERDIR}"
BBFILES += "${LAYERDIR}/recipes-*/*/*.bb \
            ${LAYERDIR}/recipes-*/*/*.bbappend"

BBFILE_COLLECTIONS += "meta-mychardev"
BBFILE_PATTERN_meta-mychardev := "^${LAYERDIR}/"
BBFILE_PRIORITY_meta-mychardev = "6"

LAYERDEPENDS_meta-mychardev = "core"
LAYERSERIES_COMPAT_meta-mychardev = "kirkstone"
```

**Step 3: Kernel Module Recipe (recipes-kernel/mychardev/mychardev.bb)**
```bitbake
SUMMARY = "Simple Character Device Driver"
DESCRIPTION = "A character device driver for Linux"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = "file://mychardev.c \
           file://Makefile \
           file://COPYING"

S = "${WORKDIR}"

do_install() {
    install -d ${D}${base_libdir}/modules/${KERNEL_VERSION}/extra
    install -m 644 ${S}/mychardev.ko ${D}${base_libdir}/modules/${KERNEL_VERSION}/extra/
}

FILES:${PN} += "${base_libdir}/modules/${KERNEL_VERSION}/extra/mychardev.ko"
RPROVIDES:${PN} += "kernel-module-mychardev"
```

**Step 4: Kernel Config (recipes-kernel/linux/linux-yocto_%.bbappend)**
```bitbake
FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://mychardev.cfg"

do_configure:append() {
    echo 'CONFIG_MYCHARDEV=m' >> ${B}/.config
}
```

**Step 5: Device Node Creation Script (recipes-example/mychardev-init/mychardev-init)**
```bash
#!/bin/sh
# mychardev-init - init script for mychardev device node

MAJOR=$(grep mychardev /proc/devices | awk '{print $1}')

case "$1" in
    start)
        echo "Creating /dev/mychardev with major $MAJOR"
        mknod /dev/mychardev c $MAJOR 0
        chmod 666 /dev/mychardev
        ;;
    stop)
        echo "Removing /dev/mychardev"
        rm -f /dev/mychardev
        ;;
    *)
        echo "Usage: $0 {start|stop}"
        exit 1
        ;;
esac

exit 0
```

**Step 6: Device Node Recipe (recipes-example/mychardev-init/mychardev-init.bb)**
```bitbake
SUMMARY = "Init script for mychardev device node"
LICENSE = "MIT"

inherit update-rc.d

INITSCRIPT_NAME = "mychardev-init"
INITSCRIPT_PARAMS = "start 99 S ."

SRC_URI = "file://mychardev-init"

do_install() {
    install -d ${D}${sysconfdir}/init.d
    install -m 755 ${WORKDIR}/mychardev-init ${D}${sysconfdir}/init.d/
}

FILES:${PN} += "${sysconfdir}/init.d/mychardev-init"
```

**Step 7: Build with Yocto**
```bash
# Source Yocto environment
source poky/oe-init-build-env build

# Add layer
bitbake-layers add-layer ../meta-mychardev

# Build image with driver
echo 'IMAGE_INSTALL:append = " mychardev mychardev-init"' >> conf/local.conf

# Build the image
bitbake core-image-minimal

# Run with QEMU
runqemu qemux86-64
```

## Prerequisites

- **Linux Kernel Headers** (`linux-headers-$(uname -r)`)
- **Build Essentials** (gcc, make)
- **QEMU** (for Yocto testing)
- **Yocto Project** (for embedded integration)

## Quick Start

```bash
# Clone repository
git clone https://github.com/yourusername/simple-char-device-driver.git
cd simple-char-device-driver

# Build driver
make

# Check build output
ls -la src/mychardev.ko
```

## Procedure

### Step 1: Build the Driver Module
```bash
cd src
make clean
make
```

### Step 2: Load Driver into Kernel
```bash
# Insert module
sudo insmod mychardev.ko

# Verify loading
lsmod | grep mychardev

# Check kernel messages
dmesg | tail -10
```

**Expected Output:**
```
[12345.678901] mychardev: Driver initialized
[12345.678902] mychardev: Major number: 240
```

### Step 3: Create Device Node
```bash
# Get major number from dmesg (e.g., 240)
MAJOR=$(dmesg | grep "Major number" | tail -1 | awk '{print $NF}')

# Create device node
sudo mknod /dev/mychardev c $MAJOR 0

# Set permissions
sudo chmod 666 /dev/mychardev

# Verify
ls -l /dev/mychardev
```

### Step 4: Test Driver
```bash
# Build user application
gcc -o user_app user_app.c

# Run test
./user_app
```

**Test Output:**
```
Opening device /dev/mychardev...
Device opened successfully

Writing data to device...
Wrote 12 bytes: Hello Kernel!

Reading data from device...
Read 12 bytes: Hello Kernel!

Closing device...
Device closed
```

### Step 5: Cleanup
```bash
# Remove device node
sudo rm /dev/mychardev

# Unload driver
sudo rmmod mychardev

# Verify removal
dmesg | tail -5
```

## Testing

### Performance Test
```bash
# Run multiple iterations
for i in {1..100}; do
    echo "Test $i" > /dev/mychardev
    cat /dev/mychardev
done
```

### Stress Testing
```bash
# Install stress tool
sudo apt-get install stress

# Stress test the driver
stress --io 4 --timeout 30s &
./user_app
```

## Troubleshooting

| Issue | Solution |
|-------|----------|
| `insmod: ERROR: could not insert module` | Check kernel version compatibility with `uname -r` |
| `Permission denied` on /dev/mychardev | Run `sudo chmod 666 /dev/mychardev` |
| Device node not found | Create node with correct major number |
| Module not loading | Check dmesg for errors: `dmesg \| grep mychardev` |
| Yocto build fails | Ensure layer dependencies are met |

## Project Structure

```
simple-char-device-driver/
├── src/
│   ├── mychardev.c        # Character device driver
│   ├── Makefile           # Build configuration
│   └── user_app.c         # Userspace test application
├── yocto/                  # Yocto recipes
│   └── meta-mychardev/
│       ├── conf/
│       │   └── layer.conf
│       ├── recipes-kernel/
│       │   ├── linux/
│       │   │   └── linux-yocto_%.bbappend
│       │   └── mychardev/
│       │       ├── mychardev.bb
│       │       ├── mychardev.c
│       │       └── Makefile
│       └── recipes-example/
│           └── mychardev-init/
│               ├── mychardev-init.bb
│               └── mychardev-init
├── Makefile               # Top-level makefile
└── README.md             # Documentation
```

## Contributing

1. Fork the repository
2. Create feature branch (`git checkout -b feature/amazing`)
3. Commit changes (`git commit -m 'Add amazing feature'`)
4. Push to branch (`git push origin feature/amazing`)
5. Open Pull Request

## License

This project is licensed under the GPLv3 License.

## Acknowledgments

- Linux Kernel Development Community
- Yocto Project Documentation
- LWN.net Character Device Drivers Tutorial

---

<div align="center">
Made with ❤️ for Linux Kernel Development
</div>
