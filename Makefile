CC=gcc
CFLAGS=-Wall -Wextra -O3 -march=native -mtune=native

.PHONY: default
default: test

.PHONY: all
all: test

test: test.c rb_tree.h
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f test
