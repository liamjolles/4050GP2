CC     = cc
CFLAGS = -std=c11 -O2 -Wall -Wextra -pedantic

all: test_inversions bench_inversions

test_inversions: test_inversions.c inversions.c inversions.h
	$(CC) $(CFLAGS) -o $@ test_inversions.c inversions.c

bench_inversions: bench_inversions.c inversions.c inversions.h
	$(CC) $(CFLAGS) -o $@ bench_inversions.c inversions.c

test: test_inversions
	./test_inversions

bench: bench_inversions
	mkdir -p results
	./bench_inversions

clean:
	rm -f test_inversions bench_inversions

.PHONY: all test bench clean
