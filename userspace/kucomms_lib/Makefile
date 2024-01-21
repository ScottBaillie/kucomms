
PWD := $(shell pwd)

TOP_DIR=$(PWD)/../..
COMMON_DIR=$(TOP_DIR)/common
USPACE_DIR=$(TOP_DIR)/userspace
USPACE_LIB_DIR=$(USPACE_DIR)/kucomms_lib
KMOD_DIR=$(TOP_DIR)/kernel-modules
KMOD_KUCOMMS_DIR=$(KMOD_DIR)/kucomms


all:
	cp $(COMMON_DIR)/message_manager.c .
	cp $(COMMON_DIR)/message_queue.c .
	gcc -c -I$(PWD) -I$(COMMON_DIR) *.c
	g++ -c -I$(PWD) -I$(COMMON_DIR) *.cpp
	ar rvs libkucomms.a *.o

clean:
	rm -f *.o
	rm -f message_manager.c
	rm -f message_queue.c
	rm -f libkucomms.a
 
