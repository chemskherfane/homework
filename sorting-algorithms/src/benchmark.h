#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "utils.h"

/* O(n^2) algorithms are skipped in comparison mode above this size.
 * Raise it (e.g. to 100000) if your machine is fast and you are patient. */
#define QUADRATIC_LIMIT 50000

typedef void (*sort_fn)(int *, int);

typedef struct {
    const char *name;
    sort_fn fn;
    int quadratic; /* 1 if O(n^2) */
    int bonus;     /* 1 if not one of the five required algorithms */
} Algo;

#define NUM_ALGOS 6 /* 5 required + built-in qsort (bonus) */
extern const Algo ALGOS[NUM_ALGOS];

typedef struct {
    double wall_ms; /* average wall-clock time */
    double cpu_ms;  /* average CPU time */
    double mem_kb;  /* peak extra memory during the sort */
} Result;

typedef struct {
    const char *algorithm;
    int size;
    Kind kind;
    int skipped;
    Result res;
    int rank; /* 1 = fastest among the five required, 0 = not ranked */
} Row;

int default_runs(int n);
Result measure(sort_fn fn, const int *data, int n, Kind kind, int runs);

extern const char *benchmark_self_path; /* argv[0], used to re-run ourselves */
int memory_child_main(int algo, int n, int kind); /* hidden --measure-memory mode */

/* Run all algorithms on identical data for each size.
 * rows must hold nsizes * NUM_ALGOS entries. Returns number of rows. */
int compare(const int *sizes, int nsizes, Kind kind, int include_builtin, Row *rows);

void print_table(const Row *rows, int nrows);
int save_csv(const Row *rows, int nrows, const char *path);

#endif
