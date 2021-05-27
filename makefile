.PHONY : all clean

cc = arm-none-linux-gnueabi-g++
include = ../Include

src = $(wildcard *.cc)
obj = $(patsubst %.cc, %, $(src))

all : $(obj)

% : %.cc
	$(cc) -o $@ $^ -g -Wall -pthread

clean:
	rm -rf $(obj)
