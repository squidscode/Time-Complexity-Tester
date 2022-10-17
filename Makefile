GCC= g++
FLAGS= -g -o $@ -std=c++11
SRCS= $(wildcard *.cpp)

all: time_test_functions

%: %.cpp
	g++ -std=c++11 -o ./test/test -g time_complexity.cpp test/time_test_functions.cpp

PHONY: clean test compile

compile:
	make clean; make all;

clean:
	rm time_test_functions;

test:
