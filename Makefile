CC=gcc
DEBUG=-g
CFLAGS=-Os -Wall -std=c11 $(DEBUG)

all: zet.c
	$(CC) zet.c -o bin/zet $(CFLAGS)

clean:
	rm -rf *.o
	rm -rf bin/*