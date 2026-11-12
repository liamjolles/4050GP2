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

## Part 2: Coin Change

| File | Contents |
|---|---|
| `coins.c` / `coins.h` | `greedy`, `naive`, `memo`, `bottom_up` (common `CoinResult` interface, `IMPOSSIBLE`), `valid_result`, `CoinStats` instrumentation |
| `test_coins.c` | 12 hand tests, 500 random cross-checks of the three exact solvers, automated greedy counterexample search |
| `bench_coins.c` | Experiment 1 (naive vs memo call counts) and Experiment 2 (bottom-up pseudo-polynomial scaling) |

Reproduce:

```sh
make test     # also writes results/greedy_counterexamples.csv
make bench    # also writes results/coins_memo.csv, results/coins_scaling.csv
```

- Random instances use `srand(4050)`: k = 1..4 distinct coins in 1..12 (3/4 of them include 1), A = 0..30.
- Greedy search: every system {1,a,b} and {1,a,b,c} with coins up to 15, A = 1..2·max coin, compared against `bottom_up`. The smallest failing A per system is written to the CSV.
- Experiment 1: D = {1,3,4}, A = 4, 8, ..., 80. Safety cutoff: once a naive run takes over 1 s, naive is skipped for all larger A.
- Experiment 2: D = {1,5,10,25}, A = 2^8 ... 2^24.
- Call/state counts come from one instrumented run; times are separate uninstrumented runs, median of 5 (1 if a single run exceeds 1 s), batched when one call is under 1 ms. The whole `bench_coins` run takes about 20 s on an M4.
