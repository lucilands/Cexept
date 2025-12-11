CFLAGS=-Wall -Wextra -pedantic -std=c99 -rdynamic
LDFLAGS=-ldl


all: demo.c bin
	$(CC) $(CFLAGS) $< -o bin/demo $(LDFLAGS)

bin:
	mkdir -p $@
