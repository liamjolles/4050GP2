#ifndef COINS_H
#define COINS_H

/* Coin change: denominations d[0..k) (distinct, positive, any order),
 * unlimited supply, target A >= 0. Find the fewest coins summing to A.
 *
 * All four solvers share one interface. On success count is the number of
 * coins and coins[0..count) lists them (malloc'd, caller frees with
 * free_result). If no solution exists count is IMPOSSIBLE and coins is NULL. */

#define IMPOSSIBLE (-1)

typedef struct {
    int count;
    int *coins;
} CoinResult;

/* Instrumentation for Part 2C. Pass NULL to skip.
 *   calls    - recursive calls (naive, memo)
 *   distinct - distinct target amounts solved (naive, memo)
 *   hits     - memo cache hits (memo)
 *   states   - DP table entries filled (bottom_up)
 *   checks   - coin comparisons in the inner loop (bottom_up) */
typedef struct {
    long long calls, distinct, hits, states, checks;
} CoinStats;

/* Part 2A-A. Largest coin <= remaining, repeat. Heuristic, not always optimal,
 * and can report IMPOSSIBLE even when a solution exists (e.g. {3,5}, A=9). */
CoinResult greedy(const int *d, int k, int A);

/* Part 2A-B. opt(r) = 1 + min over d_i <= r of opt(r - d_i), no cache.
 * Exponential, small A only. */
CoinResult naive(const int *d, int k, int A, CoinStats *st);

/* Part 2A-C. Same recurrence, each amount cached the first time it is solved. */
CoinResult memo(const int *d, int k, int A, CoinStats *st);

/* Part 2A-D. Fills opt[0..A] in increasing order. Theta(kA). */
CoinResult bottom_up(const int *d, int k, int A, CoinStats *st);

/* 1 if r is a feasible answer: count >= 0, every listed coin is in d, and
 * the count coins sum to A. Says nothing about optimality. IMPOSSIBLE passes. */
int valid_result(CoinResult r, const int *d, int k, int A);

void free_result(CoinResult r);

#endif
