
ROOT = ../..
SYMBOLIC4 = $(ROOT)/symbolic4

CC = clang++
INCLUDE_PATHS = -I$(SYMBOLIC4) -I$(SYMBOLIC4)/src -I/usr/local/include/
LIBRARY_PATHS = -L$(SYMBOLIC4)
LIBRARIES = -lm -lsymbolic4 -lgtest
CFLAGS = $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(LIBRARIES) -std=c++17 -g -Wall

.PHONY: all unit_testing

all: unit_testing run

unit_testing:
	cd $(SYMBOLIC4); \
	make
	$(CC) -o main src/*.cpp main.cpp $(CFLAGS)
	
run:
	./main
