KDIR ?= /lib/modules/$(shell uname -r)/build

# Our module name is 'my_module'
# We list the object files for each C file we want to compile into this module
obj-m := rootkit.o
rootkit-objs := src/main.o src/hide_file.o src/hide_rootkit.o


# Default rule: build the module(s)
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	rm src/.*.o.cmd src/*.o .*.cmd Module.symvers  rootkit.mod.o rootkit.mod.c rootkit.mod modules.order rootkit.o
	

# Clean rule: clean up built files
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	bear -- make
	python3 fix_compile_commands_for_nvim.py
