
PROJECT_NAME=longtest

PWD := $(shell pwd)

all:
	g++ -c -I$(PWD) *.cpp
	g++ -o kucomms_lib_$(PROJECT_NAME) *.o -lkucomms

clean:
	rm -f *.o
	rm -f kucomms_lib_$(PROJECT_NAME)

 
