
PWD := $(shell pwd)

TOP_DIR=$(PWD)/../..
COMMON_DIR=$(TOP_DIR)/common

ccflags-y += -I$(src)/../../common
ccflags-y += -I$(src)

IDIR=/lib/modules/$(shell uname -r)/kernel/drivers/char/

obj-m += kucomms.o
kucomms-objs := kucomms_mod.o kucomms_fops.o message_queue.o message_manager.o kucomms_register.o

all:
	cp $(COMMON_DIR)/message_manager.c .
	cp $(COMMON_DIR)/message_queue.c .
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f message_manager.c
	rm -f message_queue.c

install:
	install -v -m 644 kucomms.ko $(IDIR)
	depmod -a

