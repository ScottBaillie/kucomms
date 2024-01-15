
PWD := $(shell pwd)

TOP_DIR=$(PWD)/../../../..
COMMON_DIR=$(TOP_DIR)/common

ccflags-y += -I$(src)/../../../../common
ccflags-y += -I$(src)/../../../../kernel-modules/kucomms
ccflags-y += -I$(src)

IDIR=/lib/modules/$(shell uname -r)/kernel/drivers/char/

KBUILD_EXTRA_SYMBOLS=$(src)/../../../../kernel-modules/kucomms/Module.symvers

obj-m += kucomms_tutorial.o
kucomms_tutorial-objs := kucomms_tutorial_mod.o message_queue.o

all:
	cp $(COMMON_DIR)/message_queue.c .
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f message_queue.c

install:
	install -v -m 644 kucomms_tutorial.ko $(IDIR)
	depmod -a

