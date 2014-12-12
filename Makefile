CC=gcc
CFLAGS= -Wall -Werror -framework ApplicationServices

.PHONY: all

all: listen

listen: listen.c
	$(CC) $(CFLAGS) -o $@ $<
