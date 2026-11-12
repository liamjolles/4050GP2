CC     = cc
CFLAGS = -std=c11 -O2 -Wall -Wextra -pedantic

all: test_inversions bench_inversions test_coins bench_coins

test_inversions: test_inversions.c inversions.c inversions.h
	$(CC) $(CFLAGS) -o $@ test_inversions.c inversions.c

bench_inversions: bench_inversions.c inversions.c inversions.h
	$(CC) $(CFLAGS) -o $@ bench_inversions.c inversions.c

test_coins: test_coins.c coins.c coins.h
	$(CC) $(CFLAGS) -o $@ test_coins.c coins.c

bench_coins: bench_coins.c coins.c coins.h
	$(CC) $(CFLAGS) -o $@ bench_coins.c coins.c

test: test_inversions test_coins
	mkdir -p results
	./test_inversions
	./test_coins

bench: bench_inversions bench_coins
	mkdir -p results
	./bench_inversions
	./bench_coins

clean:
	rm -f test_inversions bench_inversions test_coins bench_coins

.PHONY: all test bench clean
