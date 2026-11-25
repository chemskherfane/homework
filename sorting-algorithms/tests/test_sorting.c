/* Correctness tests. Run with `make test`.
 * qsort() is used here ONLY as the reference answer. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/sorting.h"
#include "../src/utils.h"

typedef struct {
    const char *name;
    void (*fn)(int *, int);
} Entry;

static const Entry ALGOS[] = {
    {"bubble", bubble_sort},
    {"selection", selection_sort},
    {"insertion", insertion_sort},
    {"merge", merge_sort},
    {"quick", quick_sort},
};
#define N_ALGOS ((int)(sizeof ALGOS / sizeof ALGOS[0]))

static int failures = 0;

static int cmp_int(const void *x, const void *y)
{
    int a = *(const int *)x, b = *(const int *)y;
    return (a > b) - (a < b);
}

static void check(const Entry *e, const int *data, int n, const char *what)
{
    int *work = copy_array(data, n);
    int *ref = copy_array(data, n);
    qsort(ref, (size_t)n, sizeof(int), cmp_int);
    e->fn(work, n);
    if (memcmp(work, ref, (size_t)n * sizeof(int)) != 0) {
        printf("FAIL: %s sort, %s (n=%d)\n", e->name, what, n);
        failures++;
    }
    free(work);
    free(ref);
}

int main(void)
{
    /* edge cases */
    int e1[] = {1}, e2[] = {2, 1}, e3[] = {1, 1, 1, 1}, e4[] = {3, 2, 1};
    int e5[] = {1, 2, 3}, e6[] = {5, -3, 0, -3, 8};
    struct { int *d; int n; } cases[] = {
        {e1, 1}, {e2, 2}, {e3, 4}, {e4, 3}, {e5, 3}, {e6, 5}, {e1, 0},
    };
    for (int a = 0; a < N_ALGOS; a++)
        for (int c = 0; c < (int)(sizeof cases / sizeof cases[0]); c++)
            check(&ALGOS[a], cases[c].d, cases[c].n, "edge case");

    /* many random sizes and seeds */
    for (int a = 0; a < N_ALGOS; a++)
        for (unsigned seed = 1; seed <= 40; seed++) {
            int n = (int)(seed * 7 % 80);
            int *data = generate_array(n, KIND_RANDOM, seed);
            check(&ALGOS[a], data, n, "random");
            free(data);
        }

    /* every input type */
    for (int a = 0; a < N_ALGOS; a++)
        for (int k = 0; k < NUM_KINDS; k++) {
            int *data = generate_array(500, (Kind)k, SEED);
            check(&ALGOS[a], data, 500, KIND_NAMES[k]);
            free(data);
        }

    /* large arrays: worst cases for naive quick sort / deep recursion */
    for (int a = 3; a < N_ALGOS; a++) /* merge and quick only */
        for (int k = 0; k < NUM_KINDS; k++) {
            int *data = generate_array(200000, (Kind)k, SEED);
            check(&ALGOS[a], data, 200000, KIND_NAMES[k]);
            free(data);
        }

    if (failures) {
        printf("%d test(s) FAILED\n", failures);
        return 1;
    }
    printf("All tests passed.\n");
    return 0;
}
