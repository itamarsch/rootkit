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
	rm -f *.o *.ko *.mod *.mod.c modules.order Module.symvers .*.cmd
	rm -rf .tmp_versions
