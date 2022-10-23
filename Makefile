CC=gcc
CFLAGS=-Wall -Wextra -O3 -march=native -mtune=native

all: test static dynamic

static: librbt.a

dynamic: librbt.so

rb_tree.o: rb_tree.c rb_tree.h
	$(CC) $(CFLAGS) -c -o $@ $<

rb_tree.pic.o: rb_tree.c rb_tree.h
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

librbt.a: rb_tree.o
	ar rcs $@ $^

librbt.so: rb_tree.pic.o
	$(CC) -shared -o $@ $^

test: test.c librbt.a
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f rb_tree.o rb_tree.pic.o librbt.a librbt.so test

.PHONY: clean
