/* Part 2C experiments.
 *
 * Experiment 1 (repeated subproblems): D = {1,3,4}, A = 4, 8, ..., 80.
 *   Instrumented naive vs memo: recursive calls, distinct states, cache hits.
 *   Safety cutoff: once a naive run takes over CUTOFF_S seconds, naive is
 *   skipped for every larger A (memo keeps going).
 *   -> results/coins_memo.csv
 *
 * Experiment 2 (pseudo-polynomial): bottom_up, D = {1,5,10,25} (k = 4 fixed),
 *   A = 2^8, 2^9, ..., 2^24. Records bit length, DP states, inner-loop coin
 *   checks, median runtime.
 *   -> results/coins_scaling.csv
 *
 * Counts come from one instrumented run. Times are separate uninstrumented
 * runs, median of REPEATS samples, each sample averaged over a batch when
 * one call is too fast to time. */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "coins.h"

#define REPEATS 5
#define CUTOFF_S 1.0
#define CMP(a, b) (((a) > (b)) - ((a) < (b)))

#define E1_STEP 4
#define E1_MAX 80
#define E1_ROWS (E1_MAX / E1_STEP)
#define E2_MIN_EXP 8
#define E2_MAX_EXP 24
#define E2_ROWS (E2_MAX_EXP - E2_MIN_EXP + 1)

static const int D1[] = {1, 3, 4}, K1 = 3;
static const int D2[] = {1, 5, 10, 25}, K2 = 4;

typedef CoinResult (*Solver)(const int *, int, int, CoinStats *);

static volatile int sink;  /* so -O2 can't skip the calls */

static double now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static int cmp_double(const void *x, const void *y) {
    return CMP(*(const double *)x, *(const double *)y);
}

/* median seconds per call; batch sized from one trial call */
static double time_it(Solver f, const int *d, int k, int A) {
    double t0 = now();
    CoinResult r = f(d, k, A, NULL);
    double once = now() - t0;
    free_result(r);
    int batch = once > 1e-3 ? 1 : 1 + (int)(1e-3 / (once + 1e-9));
    int reps = once > CUTOFF_S ? 1 : REPEATS;  /* don't repeat the slow ones */

    double t[REPEATS];
    for (int i = 0; i < reps; i++) {
        double start = now();
        for (int b = 0; b < batch; b++) {
            r = f(d, k, A, NULL);
            sink = r.count;
            free_result(r);
        }
        t[i] = (now() - start) / batch;
    }
    qsort(t, reps, sizeof *t, cmp_double);
    return t[reps / 2];
}

static int bit_length(long long x) {
    int b = 0;
    while (x) {
        b++;
        x >>= 1;
    }
    return b;
}

int main(void) {
    /* ----- Experiment 1 ----- */
    CoinStats ns[E1_ROWS] = {{0}}, ms[E1_ROWS] = {{0}};
    double nt[E1_ROWS], mt[E1_ROWS];
    int naive_ran[E1_ROWS] = {0}, stopped = 0;
    for (int i = 0; i < E1_ROWS; i++) {
        int A = (i + 1) * E1_STEP;
        CoinResult r = memo(D1, K1, A, &ms[i]);
        free_result(r);
        mt[i] = time_it(memo, D1, K1, A);
        if (stopped) continue;
        r = naive(D1, K1, A, &ns[i]);
        free_result(r);
        nt[i] = time_it(naive, D1, K1, A);
        naive_ran[i] = 1;
        if (nt[i] > CUTOFF_S) stopped = 1;
    }

    printf("Experiment 1: D = {1,3,4}, naive cutoff after a run > %.1f s\n", CUTOFF_S);
    printf("%4s | %14s %8s %12s | %8s %8s %8s %12s\n", "A", "naive calls",
           "distinct", "time (s)", "memo cal", "distinct", "hits", "time (s)");
    for (int i = 0; i < E1_ROWS; i++) {
        printf("%4d | ", (i + 1) * E1_STEP);
        if (naive_ran[i])
            printf("%14lld %8lld %12.9f | ", ns[i].calls, ns[i].distinct, nt[i]);
        else
            printf("%14s %8s %12s | ", "(cutoff)", "-", "-");
        printf("%8lld %8lld %8lld %12.9f\n", ms[i].calls, ms[i].distinct,
               ms[i].hits, mt[i]);
    }

    FILE *f = fopen("results/coins_memo.csv", "w");
    if (!f) {
        perror("results/coins_memo.csv");
        return 1;
    }
    fprintf(f, "A,naive_calls,naive_distinct,naive_median_s,"
               "memo_calls,memo_distinct,memo_hits,memo_median_s\n");
    for (int i = 0; i < E1_ROWS; i++) {
        fprintf(f, "%d,", (i + 1) * E1_STEP);
        if (naive_ran[i])
            fprintf(f, "%lld,%lld,%.9f,", ns[i].calls, ns[i].distinct, nt[i]);
        else
            fprintf(f, ",,,");  /* skipped by cutoff */
        fprintf(f, "%lld,%lld,%lld,%.9f\n", ms[i].calls, ms[i].distinct,
                ms[i].hits, mt[i]);
    }
    fclose(f);
    printf("wrote results/coins_memo.csv\n\n");

    /* ----- Experiment 2 ----- */
    CoinStats bs[E2_ROWS] = {{0}};
    double bt[E2_ROWS];
    for (int i = 0; i < E2_ROWS; i++) {
        int A = 1 << (E2_MIN_EXP + i);
        CoinResult r = bottom_up(D2, K2, A, &bs[i]);
        free_result(r);
        bt[i] = time_it(bottom_up, D2, K2, A);
    }

    printf("Experiment 2: bottom-up, D = {1,5,10,25} (k = 4)\n");
    printf("%10s %5s %10s %11s %12s %9s %7s\n", "A", "bits", "states",
           "coin checks", "time (s)", "ns/state", "x prev");
    for (int i = 0; i < E2_ROWS; i++) {
        long long A = 1LL << (E2_MIN_EXP + i);
        printf("%10lld %5d %10lld %11lld %12.9f %9.2f", A, bit_length(A),
               bs[i].states, bs[i].checks, bt[i], bt[i] / bs[i].states * 1e9);
        if (i) printf(" %7.2f", bt[i] / bt[i - 1]);
        printf("\n");
    }

    f = fopen("results/coins_scaling.csv", "w");
    if (!f) {
        perror("results/coins_scaling.csv");
        return 1;
    }
    fprintf(f, "A,bit_length,dp_states,coin_checks,median_s\n");
    for (int i = 0; i < E2_ROWS; i++) {
        long long A = 1LL << (E2_MIN_EXP + i);
        fprintf(f, "%lld,%d,%lld,%lld,%.9f\n", A, bit_length(A), bs[i].states,
                bs[i].checks, bt[i]);
    }
    fclose(f);
    printf("wrote results/coins_scaling.csv (median of %d)\n", REPEATS);
    return 0;
}
