
PWD := $(shell pwd)

TOP_DIR=$(PWD)/../..
COMMON_DIR=$(TOP_DIR)/common
USPACE_DIR=$(TOP_DIR)/userspace
USPACE_LIB_DIR=$(USPACE_DIR)/kucomms
KMOD_DIR=$(TOP_DIR)/kernel-modules
KMOD_KUCOMMS_DIR=$(KMOD_DIR)/kucomms


all:
	g++ -c -I$(PWD) -I$(COMMON_DIR) -I$(USPACE_DIR) *.cpp
	g++ -o kucomms_lib_test *.o $(USPACE_LIB_DIR)/libkucomms.a

clean:
	rm -f *.o
	rm -f kucomms_lib_test

 
