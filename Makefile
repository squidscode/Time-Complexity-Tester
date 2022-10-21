GCC= g++
FLAGS= -g -o $@ -std=c++11
SRCS= $(wildcard ./test/*/main.cpp)
DEST= $(patsubst ./test/%/main.cpp,./executables/%.exe,$(SRCS))
OBJS= $(patsubst ./test/%/main.cpp,./object-files/%.o, $(SRCS))

all: compile-object-files $(DEST)

./executables/%.exe: ./object-files/%.o
	g++ -o $@ $< ./object-files/time_complexity.o ./object-files/gradient_descent.o

./object-files/%.o: ./test/%/main.cpp
	g++ -std=c++11 -c -Wall -o $@ $<	 

PHONY: clean test compile-object-files

compile-object-files:
	g++ -std=c++11 -c -Wall -o ./object-files/time_complexity.o time_complexity.cpp
	g++ -std=c++11 -c -Wall -o ./object-files/gradient_descent.o gradient_descent/gradient_descent.cpp

clean:
	rm -rfv ./object-files/*.o;
	rm -rfv ./executables/*.exe;

test:
