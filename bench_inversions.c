/* Part 1B runtime: brute force vs sort-and-count on random permutations.
 * Prints a table and writes results/inversions_bench.csv.
 *
 * Each time is the median of REPEATS samples. One call at small n is
 * below the timer's resolution, so a sample is the average over a batch
 * of calls. The input is copied before every call for both methods,
 * since sort_and_count sorts in place. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "inversions.h"

#define SEED 4050
#define REPEATS 7
#define CMP(a, b) (((a) > (b)) - ((a) < (b)))

static const int SIZES[] = {2, 4, 8, 16, 32, 64, 128, 256, 512, 1000,
                            2000, 4000, 8000, 16000, 32000};
#define NSIZES (int)(sizeof SIZES / sizeof *SIZES)

static volatile long long sink;  /* so -O2 can't skip the calls */

static double now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static int cmp_double(const void *x, const void *y) {
    return CMP(*(const double *)x, *(const double *)y);
}

/* median seconds per call */
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
    double bf[NSIZES], dc[NSIZES];
    srand(SEED);
    for (int s = 0; s < NSIZES; s++) {
        int n = SIZES[s];
        int *a = malloc(n * sizeof *a), *work = malloc(n * sizeof *work);
        random_perm(a, n);
        bf[s] = time_it(1, a, work, n);  /* same array for both */
        dc[s] = time_it(0, a, work, n);
        free(a);
        free(work);
    }

    /* output only after all timing is done */
    printf("%7s %14s %14s %9s\n", "n", "brute (s)", "d&c (s)", "brute/dc");
    for (int s = 0; s < NSIZES; s++)
        printf("%7d %14.9f %14.9f %9.2f\n", SIZES[s], bf[s], dc[s], bf[s] / dc[s]);

    FILE *f = fopen("results/inversions_bench.csv", "w");
    if (!f) {
        perror("results/inversions_bench.csv");
        return 1;
    }
    fprintf(f, "n,brute_median_s,dc_median_s\n");
    for (int s = 0; s < NSIZES; s++)
        fprintf(f, "%d,%.9f,%.9f\n", SIZES[s], bf[s], dc[s]);
    fclose(f);
    printf("wrote results/inversions_bench.csv (median of %d, seed %d)\n", REPEATS, SEED);
    return 0;
}
