import os
import fcntl
import time

SIMC_IOC_MAGIC = ord('k')
SIMC_RESET_BUFFER = 0
SIMC_GET_STATUS = 1

def ioctl(fd, cmd, arg=0):
    fcntl.ioctl(fd, cmd, arg)

# path
device = "/dev/simchardev"

# Open device
fd = os.open(device, os.O_RDWR)
print("Python client connected to simulated char device")

# Write messages
for i in range(1, 6):
    msg = f"Hello {i}"
    os.write(fd, msg.encode())
    print(f"Written: {msg}")
    time.sleep(0.5)

# Read messages
os.lseek(fd, 0, os.SEEK_SET)
for i in range(5):
    data = os.read(fd, 128)
    print(f"Read: {data.decode()}")

# Get buffer status via ioctl
buf_status = bytearray(4)
fcntl.ioctl(fd, SIMC_GET_STATUS, buf_status)
print("Buffer status (bytes):", int.from_bytes(buf_status, 'little'))

# Reset buffer
fcntl.ioctl(fd, SIMC_RESET_BUFFER)
print("Buffer reset done via ioctl")

os.close(fd)

