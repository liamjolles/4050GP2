/* Part 2A: four coin-change solvers with one interface.
 *
 * naive, memo and bottom_up all use the same recurrence:
 *   opt(0) = 0
 *   opt(r) = 1 + min over d_i <= r of opt(r - d_i)   (INF if no d_i works)
 * They differ only in how often each opt(r) gets computed. */

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "coins.h"

#define INF INT_MAX     /* opt(r) has no solution */
#define UNSOLVED (-2)   /* memo slot not filled yet */
#define CMP(a, b) (((a) > (b)) - ((a) < (b)))

static const CoinResult NO_SOLUTION = {IMPOSSIBLE, NULL};

void free_result(CoinResult r) { free(r.coins); }

/* walk choice[] back from A: choice[r] is the last coin used to make r */
static CoinResult rebuild(const int *choice, int A, int count) {
    CoinResult res = {count, malloc((count + 1) * sizeof(int))};
    for (int i = 0, r = A; r > 0; i++) {
        res.coins[i] = choice[r];
        r -= choice[r];
    }
    return res;
}

/* ---------- A. greedy ---------- */

static int desc(const void *x, const void *y) {
    return CMP(*(const int *)y, *(const int *)x);
}

CoinResult greedy(const int *d, int k, int A) {
    int *s = malloc((k + 1) * sizeof *s), *take = calloc(k + 1, sizeof *take);
    memcpy(s, d, k * sizeof *d);
    qsort(s, k, sizeof *s, desc);  /* largest first */

    /* taking the largest coin <= rem until it no longer fits is the same as
     * taking rem / s[i] of it, then moving to the next size */
    int rem = A, count = 0;
    for (int i = 0; i < k; i++) {
        take[i] = rem / s[i];
        rem -= take[i] * s[i];
        count += take[i];
    }

    CoinResult res = NO_SOLUTION;
    if (rem == 0) {  /* rem > 0: stuck, nothing left fits */
        res.count = count;
        res.coins = malloc((count + 1) * sizeof(int));
        for (int i = 0, j = 0; i < k; i++)
            for (int t = 0; t < take[i]; t++) res.coins[j++] = s[i];
    }
    free(s);
    free(take);
    return res;
}

/* ---------- B. naive recursion ---------- */

/* opt(r) straight from the recurrence. *last gets the coin that achieves it.
 * seen[] is only for counting distinct states, it never prunes anything. */
static int naive_rec(const int *d, int k, int r, int *last, CoinStats *st,
                     char *seen) {
    if (st) {
        st->calls++;
        if (!seen[r]) {
            seen[r] = 1;
            st->distinct++;
        }
    }
    if (r == 0) return 0;
    int best = INF, best_coin = 0;
    for (int i = 0; i < k; i++) {  /* try every possible last coin */
        if (d[i] > r) continue;
        int sub = naive_rec(d, k, r - d[i], NULL, st, seen);
        if (sub != INF && sub + 1 < best) {
            best = sub + 1;
            best_coin = d[i];
        }
    }
    if (last) *last = best_coin;
    return best;
}

CoinResult naive(const int *d, int k, int A, CoinStats *st) {
    char *seen = st ? calloc(A + 1, 1) : NULL;
    int c, count = naive_rec(d, k, A, &c, st, seen);
    free(seen);
    if (count == INF) return NO_SOLUTION;

    /* no table to walk, so re-solve the remainder to find each next coin.
     * Not counted in st: the stats measure solving opt(A) only. */
    CoinResult res = {count, malloc((count + 1) * sizeof(int))};
    for (int i = 0, r = A; r > 0; i++) {
        if (i > 0) naive_rec(d, k, r, &c, NULL, NULL);
        res.coins[i] = c;
        r -= c;
    }
    return res;
}

/* ---------- C. top-down memoized ---------- */

typedef struct {
    const int *d;
    int k;
    int *best, *choice;  /* best[r] == UNSOLVED until solved */
    CoinStats *st;
} Memo;

static int memo_rec(Memo *m, int r) {
    if (m->st) m->st->calls++;
    if (m->best[r] != UNSOLVED) {  /* solved before: reuse */
        if (m->st) m->st->hits++;
        return m->best[r];
    }
    if (m->st) m->st->distinct++;
    int best = r == 0 ? 0 : INF;
    for (int i = 0; i < m->k && r > 0; i++) {  /* same loop as naive */
        if (m->d[i] > r) continue;
        int sub = memo_rec(m, r - m->d[i]);
        if (sub != INF && sub + 1 < best) {
            best = sub + 1;
            m->choice[r] = m->d[i];
        }
    }
    return m->best[r] = best;
}

CoinResult memo(const int *d, int k, int A, CoinStats *st) {
    Memo m = {d, k, malloc((A + 1) * sizeof(int)), malloc((A + 1) * sizeof(int)), st};
    for (int r = 0; r <= A; r++) m.best[r] = UNSOLVED;
    int count = memo_rec(&m, A);
    CoinResult res = count == INF ? NO_SOLUTION : rebuild(m.choice, A, count);
    free(m.best);
    free(m.choice);
    return res;
}

/* ---------- D. bottom-up ---------- */

CoinResult bottom_up(const int *d, int k, int A, CoinStats *st) {
    int *opt = malloc((A + 1) * sizeof *opt), *choice = malloc((A + 1) * sizeof *choice);
    long long checks = 0;
    opt[0] = 0;
    /* increasing r, so every opt[r - d_i] is already final when we read it */
    for (int r = 1; r <= A; r++) {
        opt[r] = INF;
        for (int i = 0; i < k; i++) {
            checks++;
            if (d[i] <= r && opt[r - d[i]] != INF && opt[r - d[i]] + 1 < opt[r]) {
                opt[r] = opt[r - d[i]] + 1;
                choice[r] = d[i];
            }
        }
    }
    if (st) {
        st->states += A + 1;
        st->checks += checks;
    }
    CoinResult res = opt[A] == INF ? NO_SOLUTION : rebuild(choice, A, opt[A]);
    free(opt);
    free(choice);
    return res;
}

/* ---------- validator ---------- */

int valid_result(CoinResult r, const int *d, int k, int A) {
    if (r.count == IMPOSSIBLE) return r.coins == NULL;
    if (r.count < 0 || (r.count > 0 && !r.coins)) return 0;
    long long sum = 0;
    for (int i = 0; i < r.count; i++) {
        int in_d = 0;
        for (int j = 0; j < k; j++)
            if (r.coins[i] == d[j]) in_d = 1;
        if (!in_d) return 0;
        sum += r.coins[i];
    }
    return sum == A;
}
