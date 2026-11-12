/* Part 1A: two inversion counters + random input generator.
 *
 * brute_force is the baseline and the oracle for testing.
 * sort_and_count is mergesort that also counts inversions while merging. */

#include <stdlib.h>
#include "inversions.h"

/* every pair i < j */
long long brute_force(const int *a, int n) {
    long long count = 0;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (a[i] > a[j]) count++;
    return count;
}

/* merge sorted a[lo..mid) and a[mid..hi), return cross inversions, O(n) */
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
    /* leftovers add no inversions */
    while (i < mid) tmp[k++] = a[i++];
    while (j < hi) tmp[k++] = a[j++];
    for (k = lo; k < hi; k++) a[k] = tmp[k];
    return count;
}

/* left + right + cross, a[lo..hi) sorted after */
static long long sort_count_rec(int *a, int *tmp, int lo, int hi) {
    if (hi - lo <= 1) return 0;  /* 0 or 1 elems, no inversions */
    int mid = lo + (hi - lo) / 2;
    long long left = sort_count_rec(a, tmp, lo, mid);
    long long right = sort_count_rec(a, tmp, mid, hi);
    return left + right + merge_and_count(a, tmp, lo, mid, hi);
}

long long sort_and_count(int *a, int n) {
    int *tmp = malloc((n + 1) * sizeof *tmp);  /* one merge buffer for all calls */
    long long count = sort_count_rec(a, tmp, 0, n);
    free(tmp);
    return count;
}

/* fisher-yates shuffle of 0..n-1 */
void random_perm(int *a, int n) {
    for (int i = 0; i < n; i++) a[i] = i;
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1), t = a[i];
        a[i] = a[j];
        a[j] = t;
    }
}
