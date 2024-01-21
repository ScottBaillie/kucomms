
PWD := $(shell pwd)

TOP_DIR=$(PWD)
COMMON_DIR=$(TOP_DIR)/common
USPACE_DIR=$(TOP_DIR)/userspace
USPACE_LIB_DIR=$(USPACE_DIR)/kucomms
USPACE_LIB_TEST_DIR=$(USPACE_DIR)/kucomms_lib_test
KMOD_DIR=$(TOP_DIR)/kernel-modules
KMOD_KUCOMMS_DIR=$(KMOD_DIR)/kucomms
KMOD_KUCOMMS_TEST_DIR=$(KMOD_DIR)/kucomms_test
EXAMPLES_DIR=$(TOP_DIR)/doc/examples


INSTALL_INC_DIR=/usr/include
INSTALL_SHARE_DIR=/usr/share
INSTALL_LIB_DIR=/usr/lib64


all:
	cd $(USPACE_LIB_DIR) ; make ; cd ..
	cd $(USPACE_LIB_TEST_DIR) ; make ; cd ..
	cd $(KMOD_KUCOMMS_DIR) ; make ; cd ..
	cd $(KMOD_KUCOMMS_TEST_DIR) ; make ; cd ..

clean:
	cd $(USPACE_LIB_DIR) ; make clean; cd ..
	cd $(USPACE_LIB_TEST_DIR) ; make clean; cd ..
	cd $(KMOD_KUCOMMS_DIR) ; make clean; cd ..
	cd $(KMOD_KUCOMMS_TEST_DIR) ; make clean; cd ..

install:
	mkdir -p $(INSTALL_INC_DIR)/kucomms
	mkdir -p $(INSTALL_SHARE_DIR)/kucomms
	cp $(COMMON_DIR)/kucomms/*.h $(INSTALL_INC_DIR)/kucomms
	cp $(USPACE_LIB_DIR)/*.h $(INSTALL_INC_DIR)/kucomms
	cp $(KMOD_KUCOMMS_DIR)/kucomms_register.h $(INSTALL_INC_DIR)/kucomms
	cp $(USPACE_LIB_DIR)/libkucomms.a $(INSTALL_LIB_DIR)
	cp $(KMOD_KUCOMMS_DIR)/Module.symvers $(INSTALL_SHARE_DIR)/kucomms

uninstall:
	rm -rf $(INSTALL_INC_DIR)/kucomms
	rm -rf $(INSTALL_SHARE_DIR)/kucomms
	rm -f $(INSTALL_LIB_DIR)/libkucomms.a

depmod:
	cd $(KMOD_KUCOMMS_DIR) ; make depmod; cd ..
	cd $(KMOD_KUCOMMS_TEST_DIR) ; make depmod; cd ..

examples:
	cd $(EXAMPLES_DIR)/kernel-modules/kucomms_tutorial ; make
	cd $(EXAMPLES_DIR)/kernel-modules/kucomms_longtest ; make
	cd $(EXAMPLES_DIR)/userspace/kucomms_lib_tutorial ; make
	cd $(EXAMPLES_DIR)/userspace/kucomms_lib_longtest ; make
	cd $(PWD)

examples_clean:
	cd $(EXAMPLES_DIR)/kernel-modules/kucomms_tutorial ; make clean
	cd $(EXAMPLES_DIR)/kernel-modules/kucomms_longtest ; make clean
	cd $(EXAMPLES_DIR)/userspace/kucomms_lib_tutorial ; make clean
	cd $(EXAMPLES_DIR)/userspace/kucomms_lib_longtest ; make clean
	cd $(PWD)

