**Simple Character Device Driver**

This project shows how Linux user-space communicates with kernel-space using a character device driver.

**Project Structure**

## 📁 Project Structure

device-driver-project/
│
├── src/
│   ├── mychardev.c        # Kernel module (device driver)
│   └── user_app.c         # User-space application
│
├── Makefile               # Build kernel module + user app
└── README.md              # Documentation

---

## 🔄 Communication Flow Diagram

User App (user_app.c)
        |
        | open(), read(), write()
        v
Linux System Call Interface
        |
        v
Character Device Driver (mychardev.c)
        |
        | copy_to_user() / copy_from_user()
        v
Kernel Space

## Build

make

**Load Driver**
sudo insmod src/mychardev.ko
dmesg | tail

**Create Device Node**
sudo mknod /dev/mychardev c <major> 0
sudo chmod 666 /dev/mychardev


Get major number from dmesg

**Run User Program**
./user_app

**Unload Driver**
sudo rmmod mychardev
