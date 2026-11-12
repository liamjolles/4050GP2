# CMP_SC 4050 Group Project 2

Divide and conquer (inversion counting) + dynamic programming (coin change).

## Environment

- C11, built with Apple clang 15.0.0 (`cc -std=c11 -O2`)
- Tested on macOS 15.6.1, Apple M4
- Any C11 compiler with `clock_gettime` should work

## Part 1: Counting Inversions

| File | Contents |
|---|---|
| `inversions.c` / `inversions.h` | `brute_force`, `sort_and_count`, and `random_perm` (input generator) |
| `test_inversions.c` | 12 hand tests + 500 random cross-checks against brute force |
| `bench_inversions.c` | runtime scaling, writes `results/inversions_bench.csv` |

Reproduce:

```sh
make test     # validation
make bench    # timing table + results/inversions_bench.csv
```

Random inputs use `srand(4050)`. `rand()` differs between C libraries, so the exact arrays can differ on another OS. Each timing is the median of 7 samples. A sample is the
average over a batch of `1 + 100000/n` calls, since one call at small n is below timer
resolution. Exact times depend on the machine.
