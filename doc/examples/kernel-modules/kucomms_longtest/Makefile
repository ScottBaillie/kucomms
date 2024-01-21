
PROJECT_NAME=longtest

PWD := $(shell pwd)

ccflags-y += -I$(src)/../../../../common
ccflags-y += -I$(src)/../../../../kernel-modules/kucomms
ccflags-y += -I$(src)

IDIR=/lib/modules/$(shell uname -r)/kernel/drivers/char/

KBUILD_EXTRA_SYMBOLS=$(src)/../../../../kernel-modules/kucomms/Module.symvers

obj-m += kucomms_$(PROJECT_NAME).o
kucomms_$(PROJECT_NAME)-objs := kucomms_$(PROJECT_NAME)_mod.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	install -v -m 644 kucomms_$(PROJECT_NAME).ko $(IDIR)
	depmod -a

