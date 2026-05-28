# Simulated Character Device Driver 

A fully functional Linux kernel character device driver simulation with circular buffering, sequence tracking, and user-space interaction.

[![Linux Kernel](https://img.shields.io/badge/Linux%20Kernel-Char%20Driver-blue)](https://www.kernel.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Python](https://img.shields.io/badge/Python-3.x-green)](https://python.org)
![Hardware Bring-up](https://img.shields.io/badge/Hardware-Bring%20up-blueviolet)


##  Quick Start

### 1. Install Dependencies
```bash
sudo apt update
sudo apt install build-essential linux-headers-$(uname -r) python3
```

### 2. Build & Load Driver
```bash
# Build kernel module
make

# Load module
sudo insmod src/simulated_chardev.ko

# Check major number
dmesg | tail

# Create device node (replace 250 with actual major)
sudo mknod /dev/simchardev c 250 0
sudo chmod 666 /dev/simchardev
```

### 3. Test with Python Client
```bash
python3 user/test_client.py
```

##  Driver Workflow

```mermaid
flowchart TD
    A[User Space] <--> B[/dev/simchardev]
    B <--> C[Kernel Space Driver]
    
    subgraph C [Driver Components]
        D[Circular Buffer]
        E[Mutex Lock]
        F[Sequence Counter]
        G[Message Storage]
    end
    
    subgraph A [User Operations]
        H[write: Store data]
        I[read: Retrieve data]
        J[ioctl: Control/Status]
        K[open/close: Access]
    end
    
    H --> D
    I --> D
    J --> F
    K --> E
```

##  Features

### Kernel Module Features
-  Dynamic major number allocation
-  Circular buffer with configurable size (128 entries)
-  Thread-safe operations with mutex locks
-  Message sequencing with auto-increment
-  CRC validation for data integrity
-  IOCTL commands: `RESET_BUFFER`, `GET_STATUS`
-  Standard file operations: `open`, `read`, `write`, `release`

### User Space Features
-  Python test client with interactive menu
-  Comprehensive error handling
-  Support for all driver operations
-  Human-readable output formatting

##  Detailed Usage

### Building the Module
```bash
# Standard build
make

# Clean build artifacts
make clean
```

### Driver Management Commands
```bash
# Load module
sudo insmod src/simulated_chardev.ko

# Verify loading (check major number)
dmesg | tail -5

# List loaded modules
lsmod | grep simulated

# Remove module
sudo rmmod simulated_chardev

# Clean up device file
sudo rm /dev/simchardev
```

### Testing Commands
```bash
# Write to device
echo "Test message" | sudo tee /dev/simchardev

# Read from device
sudo cat /dev/simchardev

# Direct IOCTL calls
python3 user/test_client.py
```

##  Python Test Client

The test client provides an interactive interface:

```bash
$ python3 user/test_client.py
=================================
Simulated Char Device Test Client
=================================
1. Write message
2. Read messages
3. Reset buffer (IOCTL)
4. Get driver status (IOCTL)
5. Run full test sequence
6. Exit
Choose option: 
```

### Example Test Session
```bash
# Write multiple messages
> 1
Enter message: Hello Driver!
Message written successfully (Seq: 1)

> 1
Enter message: Second message
Message written successfully (Seq: 2)

# Read back messages
> 2
--- Messages in buffer ---
Seq 1: Hello Driver! (CRC: 0x...)
Seq 2: Second message (CRC: 0x...)

# Check driver status
> 4
Driver Status:
- Total messages: 2
- Buffer capacity: 128
- Buffer usage: 2/128
- Last sequence: 2
```

##  IOCTL Commands

| Command | Code | Description |
|---------|------|-------------|
| `RESET_BUFFER` | 0x4001 | Clear all messages, reset sequence |
| `GET_STATUS` | 0x4002 | Get buffer usage and statistics |

### Using IOCTL from Command Line
```bash
# Reset buffer
python3 -c "
import fcntl
fd = open('/dev/simchardev', 'r')
fcntl.ioctl(fd, 0x4001)
print('Buffer reset')
"

# Get status
python3 -c "
import fcntl, struct
fd = open('/dev/simchardev', 'r')
status = bytearray(16)
fcntl.ioctl(fd, 0x4002, status)
total, capacity, used, seq = struct.unpack('IIII', status)
print(f'Status: {used}/{capacity} messages, seq:{seq}')
"
```

##  Troubleshooting

### Common Issues

1. **Module fails to load:**
   ```bash
   # Check kernel headers
   uname -r
   sudo apt install linux-headers-$(uname -r)
   
   # Check make output
   make clean && make
   ```

2. **Permission denied on /dev/simchardev:**
   ```bash
   sudo chmod 666 /dev/simchardev
   # OR add your user to appropriate group
   sudo usermod -aG dialout $USER
   ```

3. **Device busy when removing:**
   ```bash
   # Check what's using it
   sudo lsof /dev/simchardev
   
   # Force close if needed
   sudo fuser -k /dev/simchardev
   sudo rmmod simulated_chardev
   ```

### Debug Messages
Check kernel logs for driver messages:
```bash
# Tail kernel messages
dmesg -w

# Filter driver messages
dmesg | grep simulated
```

##  Learning Resources

This project demonstrates:
- Linux character device driver architecture
- User-kernel space communication
- Circular buffer implementation
- Concurrency control with mutexes
- IOCTL interface design
- Kernel module development workflow

##  Quick Reference Card

```bash
# Complete workflow
make
sudo insmod src/simulated_chardev.ko
MAJOR=$(dmesg | grep "major" | tail -1 | awk '{print $NF}')
sudo mknod /dev/simchardev c $MAJOR 0
sudo chmod 666 /dev/simchardev
python3 user/test_client.py
sudo rmmod simulated_chardev
sudo rm /dev/simchardev
```
