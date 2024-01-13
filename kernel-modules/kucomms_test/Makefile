
PWD := $(shell pwd)

ccflags-y += -I$(src)/../../common
ccflags-y += -I$(src)/../../kernel-modules/kucomms
ccflags-y += -I$(src)

IDIR=/lib/modules/$(shell uname -r)/kernel/drivers/char/

KBUILD_EXTRA_SYMBOLS=$(src)/../kucomms/Module.symvers

obj-m += kucomms_test.o
kucomms_test-objs := kucomms_test_mod.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	install -v -m 644 kucomms_test.ko $(IDIR)
	depmod -a

