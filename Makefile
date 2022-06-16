CC = g++-10
CFLAGS = -Wall -Wextra -Wshadow -Wpedantic -O2 -std=c++20

.PHONY: all clean

all: sls_bench

sls_bench: bench.o sls.o 
	$(CC) $(CFLAGS) -o bench bench.o sls.o

bench.o: bench.cpp
	$(CC) $(CFLAGS) -c $< -o $@

sls.o: src/sls.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f bench bench.o sls.o
