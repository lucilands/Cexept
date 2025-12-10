CFLAGS=-Wall -Wextra -pedantic -std=c99 -g -rdynamic


all: demo.c bin
	$(CC) $(CFLAGS) $< -o bin/demo

bin:
	mkdir -p $@
