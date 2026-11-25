#ifndef UTILS_H
#define UTILS_H

#define SEED 42u

typedef enum { KIND_RANDOM, KIND_SORTED, KIND_REVERSE, KIND_DUPLICATES, NUM_KINDS } Kind;

extern const char *KIND_NAMES[NUM_KINDS];

/* Reproducible array (own xorshift32 generator: same result on every platform).
 * Caller must free(). */
int *generate_array(int n, Kind kind, unsigned seed);
int *copy_array(const int *src, int n);
int is_sorted(const int *a, int n);
void print_array(const char *label, const int *a, int n);

#endif
