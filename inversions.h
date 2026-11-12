/* Part 1 - counting inversions */
#ifndef INVERSIONS_H
#define INVERSIONS_H

/* checks every pair, Theta(n^2) */
long long brute_force(const int *a, int n);

/* mergesort + count, Theta(n log n), sorts a in place */
long long sort_and_count(int *a, int n);

#endif
