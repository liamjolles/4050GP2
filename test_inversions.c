/* Part 1B - validation, brute force is the oracle */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inversions.h"

#define SEED 4050
#define TRIALS 500
#define MAX_N 60

typedef struct { int n; int a[12]; long long expected; const char *desc; } HandTest;

/* hand-checkable cases */
static const HandTest HAND[] = {
    {0, {0}, 0, "empty"},
    {1, {7}, 0, "single"},
    {5, {1, 2, 3, 4, 5}, 0, "sorted"},
    {5, {5, 4, 3, 2, 1}, 10, "reverse, n(n-1)/2"},
    {2, {2, 1}, 1, "one swap"},
    {5, {1, 3, 2, 4, 5}, 1, "nearly sorted, adjacent swap"},
    {5, {1, 2, 5, 4, 3}, 3, "nearly sorted, tail reversed"},
    {6, {2, 1, 4, 3, 6, 5}, 3, "nearly sorted, pairs swapped"},
    {3, {3, 1, 2}, 2, "small random"},
    {5, {2, 4, 1, 3, 5}, 3, "random"},
    {6, {6, 1, 5, 2, 4, 3}, 9, "random"},
    {12, {1, 5, 4, 8, 10, 2, 6, 9, 12, 11, 3, 7}, 22, "lecture-style example"},
};

static int cmp_int(const void *x, const void *y) {
    int a = *(const int *)x, b = *(const int *)y;
    return (a > b) - (a < b);
}

/* both counts must match, d&c output must be sorted */
static int check(const int *a, int n, long long expected) {
    int dc_arr[MAX_N + 1], ref[MAX_N + 1];
    memcpy(dc_arr, a, n * sizeof *a);
    memcpy(ref, a, n * sizeof *a);
    qsort(ref, n, sizeof *ref, cmp_int);

    long long bf = brute_force(a, n);
    long long dc = sort_and_count(dc_arr, n);
    if (bf != dc) { printf("mismatch: brute=%lld dc=%lld\n", bf, dc); return 0; }
    if (memcmp(dc_arr, ref, n * sizeof *a)) { printf("d&c output not sorted\n"); return 0; }
    if (expected >= 0 && bf != expected) { printf("got %lld, expected %lld\n", bf, expected); return 0; }
    return 1;
}

int main(void) {
    int fails = 0;
    int nhand = sizeof HAND / sizeof *HAND;
    for (int t = 0; t < nhand; t++)
        if (!check(HAND[t].a, HAND[t].n, HAND[t].expected)) {
            printf("  ^ hand test %d (%s)\n", t, HAND[t].desc);
            fails++;
        }
    printf("hand tests: %d/%d passed\n", nhand - fails, nhand);

    /* random permutations of 0..n-1, distinct values */
    srand(SEED);
    int rfails = 0, a[MAX_N + 1];
    for (int t = 0; t < TRIALS; t++) {
        int n = rand() % (MAX_N + 1);
        for (int i = 0; i < n; i++) a[i] = i;
        for (int i = n - 1; i > 0; i--) {  /* fisher-yates shuffle */
            int j = rand() % (i + 1), x = a[i];
            a[i] = a[j]; a[j] = x;
        }
        if (!check(a, n, -1)) rfails++;
    }
    printf("random cross-checks: %d/%d passed (seed %d)\n", TRIALS - rfails, TRIALS, SEED);

    return (fails || rfails) ? 1 : 0;
}
