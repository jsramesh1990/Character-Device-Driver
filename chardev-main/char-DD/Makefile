obj-m += simulated_chardev.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)/src

EXTRA_CFLAGS += -I$(PWD)/../include

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

