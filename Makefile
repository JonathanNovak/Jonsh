CC=gcc 
CFLAGS=-Wall -std=c99
all: jonsh

jonsh: jonsh.c
	$(CC) $(CFLAGS) jonsh.c -o jonsh
.PHONY: clean
clean:
	rm jonsh
