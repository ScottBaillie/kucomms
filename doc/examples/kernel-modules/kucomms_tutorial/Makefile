
PROJECT_NAME=tutorial

PWD := $(shell pwd)

INSTALL_INC_DIR=/usr/include
INSTALL_SHARE_DIR=/usr/share

ccflags-y += -I$(INSTALL_INC_DIR)
ccflags-y += -I$(src)

IDIR=/lib/modules/$(shell uname -r)/kernel/drivers/char/

KBUILD_EXTRA_SYMBOLS=$(INSTALL_SHARE_DIR)/kucomms/Module.symvers

obj-m += kucomms_$(PROJECT_NAME).o
kucomms_$(PROJECT_NAME)-objs := kucomms_$(PROJECT_NAME)_mod.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

depmod:
	install -v -m 644 kucomms_$(PROJECT_NAME).ko $(IDIR)
	depmod -a

