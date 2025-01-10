# Makefile to compile a Linux kernel module and place artifacts in bin/

# Module name
MODULE_NAME := rootkit

# Kernel build directory
KDIR := /lib/modules/$(shell uname -r)/build

# The target is the kernel module
obj-m := $(MODULE_NAME).o

# Directory for build outputs
BIN_DIR := bin

# Default target to build the module
all:
	# 1) Build in the current directory (standard external module build)
	$(MAKE) -C $(KDIR) M=$(PWD) modules

	# 2) Create bin/ if it doesn't exist
	mkdir -p $(BIN_DIR)

	# 3) Move all generated build files into bin/
	#    (We use '|| true' to ignore "file not found" warnings.)
	mv -f *.ko *.o *.mod *.mod.c modules.order Module.symvers .*.cmd \
	       $(BIN_DIR) 2>/dev/null || true
	mv -f .tmp_versions $(BIN_DIR) 2>/dev/null || true

# Clean target to remove build artifacts
clean:
	rm -rf $(BIN_DIR)
