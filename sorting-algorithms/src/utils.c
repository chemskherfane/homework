#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *KIND_NAMES[NUM_KINDS] = {"random", "sorted", "reverse", "duplicates"};

static unsigned state;

static unsigned next_rand(void)
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

int *generate_array(int n, Kind kind, unsigned seed)
{
    int *a = malloc((size_t)(n > 0 ? n : 1) * sizeof(int));
    if (!a) {
        fprintf(stderr, "generate_array: out of memory\n");
        exit(1);
    }
    state = seed ? seed : 1u;
    unsigned dup_range = (unsigned)(n / 100 + 2); /* few distinct values */

    for (int i = 0; i < n; i++) {
        switch (kind) {
        case KIND_RANDOM:     a[i] = (int)(next_rand() % 1000001u); break;
        case KIND_SORTED:     a[i] = i; break;
        case KIND_REVERSE:    a[i] = n - i; break;
        case KIND_DUPLICATES: a[i] = (int)(next_rand() % dup_range); break;
        default:              a[i] = 0; break;
        }
    }
    return a;
}

int *copy_array(const int *src, int n)
{
    int *dst = malloc((size_t)(n > 0 ? n : 1) * sizeof(int));
    if (!dst) {
        fprintf(stderr, "copy_array: out of memory\n");
        exit(1);
    }
    memcpy(dst, src, (size_t)n * sizeof(int));
    return dst;
}

int is_sorted(const int *a, int n)
{
    for (int i = 0; i + 1 < n; i++)
        if (a[i] > a[i + 1])
            return 0;
    return 1;
}

void print_array(const char *label, const int *a, int n)
{
    printf("%s", label);
    for (int i = 0; i < n; i++)
        printf(" %d", a[i]);
    printf("\n");
}
