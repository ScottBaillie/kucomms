
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
	echo kucomms_test > /sys/devices/virtual/kucomms/kucomms/create_device
	modprobe kucomms_test

rmmod:
	rmmod kucomms_test
	echo kucomms_test > /sys/devices/virtual/kucomms/kucomms/remove_device
	rmmod kucomms

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

