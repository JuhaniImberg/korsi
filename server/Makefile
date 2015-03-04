CC=gcc
CFLAGS=-c -Wall -std=c99
LDFLAGS=-lpulse

all: korsi

korsi: korsi.o
	$(CC) $(LDFLAGS) korsi.o -o korsi

korsi.o: korsi.c
	gcc $(CFLAGS) korsi.c -o korsi.o

clean:
	rm *.o korsi
