#ifndef INVERSIONS_H
#define INVERSIONS_H

/* Inversion = pair i < j with a[i] > a[j]. Values assumed distinct. */

/* Part 1A-A. Checks every pair, Theta(n^2). a is not changed. */
long long brute_force(const int *a, int n);

/* Part 1A-B. Mergesort + count, Theta(n log n).
 * Sorts a in place, so the caller gets the count and the sorted array. */
long long sort_and_count(int *a, int n);

/* Fills a with a random permutation of 0..n-1 (distinct values).
 * Uses rand(), so call srand(seed) first. */
void random_perm(int *a, int n);

#endif
