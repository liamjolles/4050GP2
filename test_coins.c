/* Part 2B validation.
 *   1. hand tests with known optima
 *   2. random small instances: naive, memo and bottom_up must agree,
 *      every returned list must pass valid_result
 *   3. automated search for systems where greedy uses more coins than optimal,
 *      written to results/greedy_counterexamples.csv */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "coins.h"

#define SEED 4050
#define TRIALS 500
#define MAX_K 4
#define MAX_COIN 12   /* random instances */
#define MAX_A 30      /* random instances, small enough for naive */
#define SEARCH_MAX 15 /* greedy search: denominations up to this */

static int failures = 0;

static void print_coins(CoinResult r) {
    if (r.count == IMPOSSIBLE) {
        printf("IMPOSSIBLE");
        return;
    }
    if (r.count == 0) printf("(none)");
    for (int i = 0; i < r.count; i++) printf("%s%d", i ? "+" : "", r.coins[i]);
}

/* run the three exact solvers, return the shared count or -2 on disagreement
 * or an invalid coin list */
static int exact(const int *d, int k, int A, CoinResult *out) {
    CoinResult r[3] = {naive(d, k, A, NULL), memo(d, k, A, NULL),
                       bottom_up(d, k, A, NULL)};
    int ok = 1;
    for (int i = 0; i < 3; i++)
        if (r[i].count != r[0].count || !valid_result(r[i], d, k, A)) ok = 0;
    int count = ok ? r[0].count : -2;
    free_result(r[0]);
    free_result(r[1]);
    if (out) *out = r[2];
    else free_result(r[2]);
    return count;
}

static void check(const char *name, const int *d, int k, int A, int want,
                  int greedy_want) {
    CoinResult opt, g = greedy(d, k, A);
    int got = exact(d, k, A, &opt);
    int ok = got == want && g.count == greedy_want && valid_result(g, d, k, A);
    printf("%s  %-30s opt %2d: ", ok ? "PASS" : "FAIL", name, got);
    print_coins(opt);
    printf("   greedy: ");
    print_coins(g);
    printf("\n");
    if (!ok) failures++;
    free_result(opt);
    free_result(g);
}

#define D(...) (int[]){__VA_ARGS__}, (int)(sizeof (int[]){__VA_ARGS__} / sizeof(int))

static void hand_tests(void) {
    check("target 0", D(1, 5, 10), 0, 0, 0);
    check("impossible {2}, 3", D(2), 3, IMPOSSIBLE, IMPOSSIBLE);
    check("impossible {5,10}, 7", D(5, 10), 7, IMPOSSIBLE, IMPOSSIBLE);
    check("one coin {1,5,10,25}, 25", D(1, 5, 10, 25), 25, 1, 1);
    check("US coins {1,5,10,25}, 63", D(1, 5, 10, 25), 63, 6, 6);
    check("only 1s {1}, 10", D(1), 10, 10, 10);
    check("unsorted {25,1,10,5}, 30", D(25, 1, 10, 5), 30, 2, 2);
    check("multi-opt {1,2,3}, 4 (2+2|1+3)", D(1, 2, 3), 4, 2, 2);
    check("multi-opt {1,2,3,4}, 5", D(1, 2, 3, 4), 5, 2, 2);
    check("greedy worse {1,3,4}, 6", D(1, 3, 4), 6, 2, 3);
    check("greedy worse {1,5,6,9}, 11", D(1, 5, 6, 9), 11, 2, 3);
    check("greedy stuck {3,5}, 9", D(3, 5), 9, 3, IMPOSSIBLE);
}

/* k distinct values in 1..MAX_COIN, includes 1 if want_one */
static int random_system(int *d, int want_one) {
    int k = 1 + rand() % MAX_K, n = 0;
    if (want_one) d[n++] = 1;
    while (n < k) {
        int c = 1 + rand() % MAX_COIN, dup = 0;
        for (int i = 0; i < n; i++)
            if (d[i] == c) dup = 1;
        if (!dup) d[n++] = c;
    }
    return k;
}

static void random_tests(void) {
    int bad = 0, impossible = 0, greedy_worse = 0;
    srand(SEED);
    for (int t = 0; t < TRIALS; t++) {
        int d[MAX_K], k = random_system(d, rand() % 4 != 0); /* 3/4 include 1 */
        int A = rand() % (MAX_A + 1);
        int opt = exact(d, k, A, NULL);
        CoinResult g = greedy(d, k, A);
        if (opt == -2) bad++;
        else if (opt == IMPOSSIBLE) {
            impossible++;
            if (g.count != IMPOSSIBLE) bad++; /* greedy can't beat "no solution" */
        } else if (g.count != IMPOSSIBLE) {
            if (!valid_result(g, d, k, A) || g.count < opt) bad++;
            if (g.count > opt) greedy_worse++;
        }
        free_result(g);
    }
    printf("%s  %d random instances, k <= %d, coins <= %d, A <= %d, seed %d "
           "(%d disagree; %d impossible, %d greedy suboptimal)\n",
           bad ? "FAIL" : "PASS", TRIALS, MAX_K, MAX_COIN, MAX_A, SEED, bad,
           impossible, greedy_worse);
    failures += bad;
}

static void fprint_coins(FILE *f, CoinResult r) {
    for (int i = 0; i < r.count; i++) fprintf(f, "%s%d", i ? " " : "", r.coins[i]);
}

/* smallest A in 1..2*max(d) where greedy is feasible but uses more coins than
 * bottom_up. Returns 1 and writes a CSV row if one exists. */
static int search_system(const int *d, int k, FILE *f, int *found_A) {
    for (int A = 1; A <= 2 * d[k - 1]; A++) {
        CoinResult g = greedy(d, k, A), opt = bottom_up(d, k, A, NULL);
        int hit = g.count != IMPOSSIBLE && g.count > opt.count;
        if (hit) {
            fprintf(f, "\"");
            for (int i = 0; i < k; i++) fprintf(f, "%s%d", i ? " " : "", d[i]);
            fprintf(f, "\",%d,%d,\"", A, g.count);
            fprint_coins(f, g);
            fprintf(f, "\",%d,\"", opt.count);
            fprint_coins(f, opt);
            fprintf(f, "\"\n");
            *found_A = A;
        }
        free_result(g);
        free_result(opt);
        if (hit) return 1;
    }
    return 0;
}

/* every system {1,a,b} and {1,a,b,c} with 1 < a < b < c <= SEARCH_MAX */
static void greedy_search(void) {
    FILE *f = fopen("results/greedy_counterexamples.csv", "w");
    if (!f) {
        perror("results/greedy_counterexamples.csv");
        failures++;
        return;
    }
    fprintf(f, "denominations,A,greedy_count,greedy_coins,opt_count,opt_coins\n");
    int systems = 0, bad_systems = 0, sanity = 0;
    for (int a = 2; a <= SEARCH_MAX; a++)
        for (int b = a + 1; b <= SEARCH_MAX; b++)
            for (int c = b; c <= SEARCH_MAX; c++) { /* c == b means k = 3 */
                int d[4] = {1, a, b, c}, k = c == b ? 3 : 4, A;
                systems++;
                if (search_system(d, k, f, &A)) {
                    bad_systems++;
                    if (k == 3 && a == 3 && b == 4 && A == 6) sanity = 1;
                }
            }
    fclose(f);
    printf("PASS  greedy search: %d of %d systems have a counterexample "
           "(results/greedy_counterexamples.csv)\n", bad_systems, systems);

    printf("%s  sanity check: search found {1,3,4}, A = 6\n", sanity ? "PASS" : "FAIL");
    if (!sanity) failures++;

    /* the three examples used in the report */
    const int s1[] = {1, 3, 4}, s2[] = {1, 5, 6, 9}, s3[] = {1, 7, 10};
    const struct { const int *d; int k, A; } ex[] = {{s1, 3, 6}, {s2, 4, 11}, {s3, 3, 14}};
    for (int i = 0; i < 3; i++) {
        CoinResult g = greedy(ex[i].d, ex[i].k, ex[i].A);
        CoinResult opt = bottom_up(ex[i].d, ex[i].k, ex[i].A, NULL);
        printf("      {");
        for (int j = 0; j < ex[i].k; j++) printf("%s%d", j ? "," : "", ex[i].d[j]);
        printf("}, A = %d: greedy ", ex[i].A);
        print_coins(g);
        printf(" (%d)  optimal ", g.count);
        print_coins(opt);
        printf(" (%d)\n", opt.count);
        if (!(g.count > opt.count)) failures++;
        free_result(g);
        free_result(opt);
    }
}

int main(void) {
    hand_tests();
    random_tests();
    greedy_search();
    printf("\n%s (%d failure%s)\n", failures ? "FAILED" : "ALL PASSED", failures,
           failures == 1 ? "" : "s");
    return failures != 0;
}
