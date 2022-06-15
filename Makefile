CC = g++-10
CFLAGS = -Wall -Wextra -Wshadow -Wpedantic -O2 -std=c++20

.PHONY: all clean

all: sls_bench

sls_bench: main.o sls.o 
	$(CC) $(CFLAGS) -o main main.o sls.o

main.o: main.cpp
	$(CC) $(CFLAGS) -c $< -o $@

sls.o: src/sls.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f main main.o sls.o
