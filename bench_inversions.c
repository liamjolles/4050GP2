/* Part 1B - runtime scaling, brute force vs sort-and-count
 * writes results/inversions_bench.csv */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "inversions.h"

#define SEED 4050
#define REPEATS 7

static const int SIZES[] = {2, 4, 8, 16, 32, 64, 128, 256, 512, 1000,
                            2000, 4000, 8000, 16000, 32000};

static volatile long long sink;  /* keeps compiler from skipping calls */

static double now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static int cmp_double(const void *x, const void *y) {
    double a = *(const double *)x, b = *(const double *)y;
    return (a > b) - (a < b);
}

/* median of REPEATS samples, each sample = avg over a batch of calls
 * (timer too coarse for one call at small n), copy done for both methods */
static double time_it(int brute, const int *a, int *work, int n) {
    double t[REPEATS];
    int batch = 1 + 100000 / n;
    for (int r = 0; r < REPEATS; r++) {
        double start = now();
        for (int b = 0; b < batch; b++) {
            memcpy(work, a, n * sizeof *a);
            sink = brute ? brute_force(work, n) : sort_and_count(work, n);
        }
        t[r] = (now() - start) / batch;
    }
    qsort(t, REPEATS, sizeof *t, cmp_double);
    return t[REPEATS / 2];
}

int main(void) {
    int nsizes = sizeof SIZES / sizeof *SIZES;
    double bf[sizeof SIZES / sizeof *SIZES], dc[sizeof SIZES / sizeof *SIZES];
    srand(SEED);

    for (int s = 0; s < nsizes; s++) {
        int n = SIZES[s];
        int *a = malloc(n * sizeof *a), *work = malloc(n * sizeof *work);
        for (int i = 0; i < n; i++) a[i] = i;
        for (int i = n - 1; i > 0; i--) {  /* random permutation */
            int j = rand() % (i + 1), x = a[i];
            a[i] = a[j]; a[j] = x;
        }
        bf[s] = time_it(1, a, work, n);  /* same array for both */
        dc[s] = time_it(0, a, work, n);
        free(a); free(work);
    }

    /* printing + file output after all timing */
    printf("%7s %14s %14s %9s\n", "n", "brute (s)", "d&c (s)", "brute/dc");
    for (int s = 0; s < nsizes; s++)
        printf("%7d %14.9f %14.9f %9.2f\n", SIZES[s], bf[s], dc[s], bf[s] / dc[s]);

    FILE *f = fopen("results/inversions_bench.csv", "w");
    if (!f) { perror("results/inversions_bench.csv"); return 1; }
    fprintf(f, "n,brute_median_s,dc_median_s\n");
    for (int s = 0; s < nsizes; s++)
        fprintf(f, "%d,%.9f,%.9f\n", SIZES[s], bf[s], dc[s]);
    fclose(f);
    printf("wrote results/inversions_bench.csv (median of %d runs, seed %d)\n", REPEATS, SEED);
    return 0;
}
