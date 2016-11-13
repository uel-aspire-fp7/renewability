CC=gcc

CFLAGS=	-Wall -g -static

all: rntest

rntest: rntest.o
	$(CC) main.c rntest.o -o rntest

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm *.o rntest -f