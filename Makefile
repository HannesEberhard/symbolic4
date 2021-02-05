
ROOT = .
SYMBOLIC4 = $(ROOT)/symbolic4

CC = clang
INCLUDE_PATHS = -I$(SYMBOLIC4)
LIBRARY_PATHS = -L$(SYMBOLIC4)
LIBRARIES = -lm -lsymbolic4
CFLAGS = $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(LIBRARIES) -O3 -Wall

.PHONY: all main

all: main

main:
	cd $(SYMBOLIC4); \
	make
	$(CC) -o main main.c $(CFLAGS)
