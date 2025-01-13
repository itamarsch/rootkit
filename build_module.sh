#!/bin/bash

KDIR="/lib/modules/$(uname -r)/build"

MODULE_NAME="rootkit"

OBJ_FILES=$(find src -name '*.c' -exec basename {} .c \; | sed 's|^|src/|' | sed 's|$|.o|' | tr '\n' ' ')

cat <<EOF >Makefile
KDIR := $KDIR

obj-m := $MODULE_NAME.o

$MODULE_NAME-objs := $OBJ_FILES

all:
	\$(MAKE) -C \$(KDIR) M=\$(PWD) modules
EOF

make

mkdir -p bin
mv $MODULE_NAME.ko src/*.o src/.*.cmd .*.cmd modules.order Module.symvers $MODULE_NAME.mod.c $MODULE_NAME.mod.o $MODULE_NAME.mod $MODULE_NAME.o bin/
rm Makefile
