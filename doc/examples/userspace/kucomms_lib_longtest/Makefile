
PROJECT_NAME=longtest

PWD := $(shell pwd)

TOP_DIR=$(PWD)/../../../..
COMMON_DIR=$(TOP_DIR)/common
USPACE_DIR=$(TOP_DIR)/userspace
USPACE_LIB_DIR=$(USPACE_DIR)/kucomms_lib
KMOD_DIR=$(TOP_DIR)/kernel-modules
KMOD_KUCOMMS_DIR=$(KMOD_DIR)/kucomms


all:
	g++ -c -I$(PWD) -I$(COMMON_DIR) -I$(USPACE_LIB_DIR) *.cpp
	g++ -o kucomms_lib_$(PROJECT_NAME) *.o $(USPACE_LIB_DIR)/*.o

clean:
	rm -f *.o
	rm -f kucomms_lib_$(PROJECT_NAME)

 
