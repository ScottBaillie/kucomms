
PWD := $(shell pwd)

TOP_DIR=$(PWD)
COMMON_DIR=$(TOP_DIR)/common
USPACE_DIR=$(TOP_DIR)/userspace
USPACE_LIB_DIR=$(USPACE_DIR)/kucomms_lib
USPACE_LIB_TEST_DIR=$(USPACE_DIR)/kucomms_lib_test
KMOD_DIR=$(TOP_DIR)/kernel-modules
KMOD_KUCOMMS_DIR=$(KMOD_DIR)/kucomms
KMOD_KUCOMMS_TEST_DIR=$(KMOD_DIR)/kucomms_test


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
	cd $(KMOD_KUCOMMS_DIR) ; make install; cd ..
	cd $(KMOD_KUCOMMS_TEST_DIR) ; make install; cd ..

insmod:
	modprobe kucomms
	modprobe kucomms_test

rmmod:
	rmmod kucomms_test
	rmmod kucomms

