
.PHONY: all docs

all: docs

docs:
	rm -rf html latex
	doxygen config
	cd latex; \
	make pdf; \
	mv refman.pdf symbolic4.pdf; \
	make --always-make clean
