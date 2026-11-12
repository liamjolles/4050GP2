/* Part 1A - brute force and divide-and-conquer inversion counters */

#include <stdlib.h>
#include "inversions.h"

/* every pair i < j, count when a[i] > a[j] */
long long brute_force(const int *a, int n) {
    long long count = 0;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (a[i] > a[j]) count++;
    return count;
}

/* merge sorted a[lo..mid) and a[mid..hi), count cross inversions, O(n) */
static long long merge_and_count(int *a, int *tmp, int lo, int mid, int hi) {
    long long count = 0;
    int i = lo, j = mid, k = lo;
    while (i < mid && j < hi) {
        if (a[i] <= a[j]) {
            tmp[k++] = a[i++];
        } else {
            /* right elem smaller -> inversion w/ every remaining left elem */
            count += mid - i;
            tmp[k++] = a[j++];
        }
    }
    /* leftovers, no new inversions */
    while (i < mid) tmp[k++] = a[i++];
    while (j < hi) tmp[k++] = a[j++];
    for (k = lo; k < hi; k++) a[k] = tmp[k];
    return count;
}

/* left count + right count + cross count, a[lo..hi) sorted after */
static long long sort_count_rec(int *a, int *tmp, int lo, int hi) {
    if (hi - lo <= 1) return 0;  /* base case */
    int mid = lo + (hi - lo) / 2;
    long long left = sort_count_rec(a, tmp, lo, mid);
    long long right = sort_count_rec(a, tmp, mid, hi);
    long long cross = merge_and_count(a, tmp, lo, mid, hi);
    return left + right + cross;
}

long long sort_and_count(int *a, int n) {
    int *tmp = malloc((n + 1) * sizeof *tmp);  /* one shared merge buffer */
    long long count = sort_count_rec(a, tmp, 0, n);
    free(tmp);
    return count;
}
