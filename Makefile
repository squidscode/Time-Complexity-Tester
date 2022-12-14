GCC= g++
FLAGS= -g -o $@ -std=c++11
FILES= time_complexity.cpp gradient_descent.cpp
FOBJ= $(patsubst %.cpp, ./object-files/%.o, $(FILES))
SRCS= $(wildcard ./test/*/main.cpp)
DEST= $(patsubst ./test/%/main.cpp, ./executables/%.exe,$(SRCS))
OBJS= $(patsubst ./test/%/main.cpp, ./object-files/%.o, $(SRCS))

all: $(FOBJ) $(OBJ) $(DEST)

./executables/%.exe: ./object-files/%.o
	g++ -g -o "$@" "$<" ./object-files/time_complexity.o ./object-files/gradient_descent.o

./object-files/time_complexity.o: time_complexity.cpp
	g++ -std=c++11 -c -g -Wall -o $@ $^

./object-files/gradient_descent.o: gradient_descent/gradient_descent.cpp
	g++ -std=c++11 -c -g -Wall -o $@ $^

./object-files/%.o: ./test/%/main.cpp
	g++ -std=c++11 -c -g -Wall -o "$@" "$<"

PHONY: clean test compile-object-files compile

# Compiles the given target.
# make TARGET=... compile-file
compile: ./object-files/$(TARGET).o ./executables/$(TARGET).exe

clean:
	rm -rfv ./object-files/*.o;
	rm -rfv ./executables/*.exe;

test:
