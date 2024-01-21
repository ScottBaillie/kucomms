
PROJECT_NAME=tutorial

PWD := $(shell pwd)

TOP_DIR=$(PWD)/../../../..
COMMON_DIR=$(TOP_DIR)/common

ccflags-y += -I$(src)/../../../../common
ccflags-y += -I$(src)/../../../../kernel-modules/kucomms
ccflags-y += -I$(src)
ccflags-y += -DKERNEL_BUILD=1

IDIR=/lib/modules/$(shell uname -r)/kernel/drivers/char/

KBUILD_EXTRA_SYMBOLS=$(src)/../../../../kernel-modules/kucomms/Module.symvers

obj-m += kucomms_$(PROJECT_NAME).o
kucomms_$(PROJECT_NAME)-objs := kucomms_$(PROJECT_NAME)_mod.o message_queue.o

all:
	cp $(COMMON_DIR)/message_queue.c .
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f message_queue.c

install:
	install -v -m 644 kucomms_$(PROJECT_NAME).ko $(IDIR)
	depmod -a

