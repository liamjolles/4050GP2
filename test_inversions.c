/* Part 1B validation: brute_force is the oracle for sort_and_count.
 * Hand tests check against a known answer, then 500 random arrays
 * check that the two counters agree and the output is sorted. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inversions.h"

#define SEED 4050
#define TRIALS 500
#define MAX_N 60

static int failures = 0;

/* both counts match (and match want, if want >= 0), output sorted */
static int agree(const int *a, int n, long long want) {
    int b[MAX_N + 1];
    memcpy(b, a, n * sizeof *a);
    long long bf = brute_force(a, n);
    long long dc = sort_and_count(b, n);
    int ok = (bf == dc) && (want < 0 || bf == want);
    for (int i = 1; i < n; i++)
        if (b[i - 1] > b[i]) ok = 0;
    return ok;
}

static void check(const char *name, const int *a, int n, long long want) {
    int ok = agree(a, n, want);
    printf("%s  %s (want %lld)\n", ok ? "PASS" : "FAIL", name, want);
    if (!ok) failures++;
}

int main(void) {
    check("empty                       ", (int[]){0}, 0, 0);
    check("single                      ", (int[]){7}, 1, 0);
    check("sorted                      ", (int[]){1, 2, 3, 4, 5}, 5, 0);
    check("reverse sorted, 5*4/2       ", (int[]){5, 4, 3, 2, 1}, 5, 10);
    check("two elems swapped           ", (int[]){2, 1}, 2, 1);
    check("nearly sorted, one swap     ", (int[]){1, 3, 2, 4, 5}, 5, 1);
    check("nearly sorted, tail reversed", (int[]){1, 2, 5, 4, 3}, 5, 3);
    check("nearly sorted, pairs swapped", (int[]){2, 1, 4, 3, 6, 5}, 6, 3);
    check("random order, n=3           ", (int[]){3, 1, 2}, 3, 2);
    check("random order, n=5           ", (int[]){2, 4, 1, 3, 5}, 5, 3);
    check("random order, n=6           ", (int[]){6, 1, 5, 2, 4, 3}, 6, 9);
    check("random order, n=12          ",
          (int[]){1, 5, 4, 8, 10, 2, 6, 9, 12, 11, 3, 7}, 12, 22);

    /* random permutations, small enough for brute force */
    srand(SEED);
    int a[MAX_N + 1], rand_fails = 0;
    for (int t = 0; t < TRIALS; t++) {
        int n = rand() % (MAX_N + 1);
        random_perm(a, n);
        if (!agree(a, n, -1)) rand_fails++;
    }
    printf("%s  %d random arrays, n = 0..%d, seed %d (%d disagree)\n",
           rand_fails ? "FAIL" : "PASS", TRIALS, MAX_N, SEED, rand_fails);
    failures += rand_fails;

    printf("\n%s (%d failure%s)\n", failures ? "FAILED" : "ALL PASSED",
           failures, failures == 1 ? "" : "s");
    return failures != 0;
}
