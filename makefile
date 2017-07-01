
CC=gcc
CFLAGS="-Wall"

all: bas2tap

bas2tap: bas2tap.o

bas2tap.o: bas2tap.c

clean:
	rm -f bas2tap.o bas2tap
