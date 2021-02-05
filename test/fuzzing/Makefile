
ROOT = ../..
SYMBOLIC4 = $(ROOT)/symbolic4

CC = clang++
INCLUDE_PATHS = -I$(SYMBOLIC4) -I$(SYMBOLIC4)/src
LIBRARY_PATHS = -L$(SYMBOLIC4)
LIBRARIES = -lm -lsymbolic4
CFLAGS = $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(LIBRARIES) -std=c++17 -g -Wall -fsanitize=fuzzer

.PHONY: all fuzzing

all: fuzzing run

fuzzing:
	mkdir -p corpus crashes
	cd $(SYMBOLIC4); \
	make
	$(CC) -o main main.cpp $(CFLAGS)
	
run:
	./main corpus -max_len=25 -rss_limit_mb=0; \
	mv crash-* crashes

merge:
	mkdir temp
	./main -merge=1 temp corpus
	rm -rf corpus
	mv temp corpus
