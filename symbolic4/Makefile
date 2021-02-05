ROOT = ../..
SYMBOLIC4 = $(ROOT)/symbolic4

CC = clang
INCLUDE_PATHS = -I$(SYMBOLIC4)
LIBRARY_PATHS =
LIBRARIES = -lm
CFLAGS = $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(LIBRARIES) -O3 -Wall -c

.PHONY: all libsymbolic4

all: libsymbolic4

libsymbolic4:
	mkdir -p build
	cd build; \
	$(CC) $(SYMBOLIC4)/src/*.c $(CFLAGS); \
	ar -rc libsymbolic4.a *.o
	mv build/libsymbolic4.a libsymbolic4.a
	rm -rf build
