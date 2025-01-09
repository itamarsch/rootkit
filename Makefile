# Makefile to compile a Linux kernel module

# Module name
MODULE_NAME := rootkit

# Kernel build directory
KDIR := /lib/modules/$(shell uname -r)/build

# The target is the kernel module
obj-m := $(MODULE_NAME).o

# Default target to build the module
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Clean target to remove build artifacts
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
