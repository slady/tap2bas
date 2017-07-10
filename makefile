
CC=gcc
CFLAGS="-Wall"

.PHONY : test all clean %.test

all: tap2bas

tap2bas: tap2bas.o

tap2bas.o: tap2bas.c

clean:
	rm -f tap2bas.o tap2bas

all-tests := $(addsuffix .test, $(basename $(wildcard test/*.tap)))

test: clean all $(all-tests)

%.test: %.tap %.bas
	./tap2bas $(word 1, $?) | diff $(word 2, $?) - || (echo "Test $@ failed" && exit 1)
