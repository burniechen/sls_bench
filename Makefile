CC = gcc
CFLAGS = -Wall -Wextra -Wshadow -Wpedantic -O2

.PHONY: all clean

all: sls_bench

sls_bench: test.o sls.o 
	$(CC) $(CFLAGS) -o test test.o sls.o

test.o: test.c
	$(CC) $(CFLAGS) -c $< -o $@

sls.o: src/sls.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f test test.o sls.o
