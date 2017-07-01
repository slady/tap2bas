
CC=gcc
CFLAGS="-Wall"

all: tap2bas

tap2bas: tap2bas.o

tap2bas.o: tap2bas.c

clean:
	rm -f tap2bas.o tap2bas
