CC=gcc
CFLAGS= -Wall -Werror -I/usr/local/include

LDFLAGS= -framework ApplicationServices -L/usr/local/lib -lev

.PHONY: all

all: listen

listen: listen.o
	$(CC) $(LDFLAGS) $< -o $@

listen.o: listen.c
	$(CC) $(CFLAGS) -c -o $@ $<
