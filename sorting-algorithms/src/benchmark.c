/* Time and memory measurement (POSIX: Linux, macOS, WSL).
 *
 *  - clock_gettime(CLOCK_MONOTONIC) : wall-clock time
 *  - clock()                        : CPU (processor) time
 *  - getrusage() ru_maxrss          : peak resident memory
 *
 * ru_maxrss is a peak for the whole process and never decreases, so the
 * memory of each sort is measured in a freshly started copy of this program
 * (--measure-memory): it builds the data, reads its peak RSS, sorts, reads the
 * peak again; the difference is the extra memory the sort needed. Timing is
 * done in the parent, in separate runs, so process start-up and page faults
 * never pollute the times. */

#define _POSIX_C_SOURCE 200809L

#include "benchmark.h"
#include "sorting.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/* ---- bonus: library qsort, only used for the optional comparison ---- */
static int cmp_int(const void *x, const void *y)
{
    int a = *(const int *)x, b = *(const int *)y;
    return (a > b) - (a < b);
}

static void builtin_sort(int *a, int n)
{
    qsort(a, (size_t)n, sizeof(int), cmp_int);
}

const Algo ALGOS[NUM_ALGOS] = {
    {"Bubble Sort",           bubble_sort,    1, 0},
    {"Selection Sort",        selection_sort, 1, 0},
    {"Insertion Sort",        insertion_sort, 1, 0},
    {"Merge Sort",            merge_sort,     0, 0},
    {"Quick Sort",            quick_sort,     0, 0},
    {"C qsort() (bonus)",     builtin_sort,   0, 1},
};

int default_runs(int n)
{
    if (n <= 10000) return 5;
    if (n <= 50000) return 3;
    return 1;
}

static double elapsed_ms(struct timespec a, struct timespec b)
{
    return (double)(b.tv_sec - a.tv_sec) * 1000.0 + (double)(b.tv_nsec - a.tv_nsec) / 1e6;
}

static long peak_rss_kb(void)
{
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
#ifdef __APPLE__
    return ru.ru_maxrss / 1024; /* bytes on macOS */
#else
    return ru.ru_maxrss;        /* kilobytes on Linux */
#endif
}

const char *benchmark_self_path = "./build/sorting";

/* Child side (hidden option --measure-memory): runs in a FRESH process so the
 * baseline peak is small and clean. Prints the extra peak memory in KB. */
int memory_child_main(int algo, int n, int kind)
{
    if (algo < 0 || algo >= NUM_ALGOS || kind < 0 || kind >= NUM_KINDS || n < 0)
        return 1;
    int *data = generate_array(n, (Kind)kind, SEED);
    int *work = copy_array(data, n);
    long before = peak_rss_kb();
    ALGOS[algo].fn(work, n);
    long after = peak_rss_kb();
    printf("%ld\n", after - before);
    free(data);
    free(work);
    return 0;
}

static double measure_memory_kb(sort_fn fn, int n, Kind kind)
{
    int algo = -1;
    for (int i = 0; i < NUM_ALGOS; i++)
        if (ALGOS[i].fn == fn)
            algo = i;
    if (algo < 0)
        return -1.0;

    /* resolve our own path here: "/proc/self/exe" inside popen would be the shell */
    char self_buf[512];
    const char *self = benchmark_self_path;
    ssize_t len = readlink("/proc/self/exe", self_buf, sizeof self_buf - 1);
    if (len > 0) {
        self_buf[len] = '\0';
        self = self_buf;
    }
    char cmd[1024];
    snprintf(cmd, sizeof cmd, "\"%s\" --measure-memory %d %d %d", self, algo, n, (int)kind);
    FILE *p = popen(cmd, "r");
    if (!p)
        return -1.0;
    long kb = -1;
    if (fscanf(p, "%ld", &kb) != 1)
        kb = -1;
    pclose(p);
    return (double)kb;
}

Result measure(sort_fn fn, const int *data, int n, Kind kind, int runs)
{
    Result r = {0, 0, 0};
    int *work = copy_array(data, n);
    double wall = 0, cpu = 0;

    for (int k = 0; k < runs; k++) {
        memcpy(work, data, (size_t)n * sizeof(int)); /* fresh copy, not timed */
        struct timespec t0, t1;
        clock_t c0 = clock();
        clock_gettime(CLOCK_MONOTONIC, &t0);
        fn(work, n);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        clock_t c1 = clock();
        wall += elapsed_ms(t0, t1);
        cpu += (double)(c1 - c0) * 1000.0 / CLOCKS_PER_SEC;
        if (!is_sorted(work, n)) {
            fprintf(stderr, "ERROR: sort produced an unsorted result\n");
            exit(1);
        }
    }
    free(work);

    r.wall_ms = wall / runs;
    r.cpu_ms = cpu / runs;
    r.mem_kb = measure_memory_kb(fn, n, kind);
    return r;
}

int compare(const int *sizes, int nsizes, Kind kind, int include_builtin, Row *rows)
{
    int count = 0;
    for (int s = 0; s < nsizes; s++) {
        int n = sizes[s];
        int *data = generate_array(n, kind, SEED);
        int first = count;

        for (int a = 0; a < NUM_ALGOS; a++) {
            if (ALGOS[a].bonus && !include_builtin)
                continue;
            Row *row = &rows[count++];
            memset(row, 0, sizeof *row);
            row->algorithm = ALGOS[a].name;
            row->size = n;
            row->kind = kind;
            if (ALGOS[a].quadratic && n > QUADRATIC_LIMIT) {
                row->skipped = 1;
                printf("  %s skipped for n=%d (too slow, see README)\n", ALGOS[a].name, n);
            } else {
                printf("  running %s on n=%d (%s)...\n", ALGOS[a].name, n, KIND_NAMES[kind]);
                fflush(stdout);
                row->res = measure(ALGOS[a].fn, data, n, kind, default_runs(n));
            }
        }
        free(data);

        /* rank the five required algorithms by wall time (1 = fastest) */
        for (int i = first; i < count; i++) {
            if (rows[i].skipped || rows[i].algorithm == ALGOS[5].name)
                continue;
            int rank = 1;
            for (int j = first; j < count; j++) {
                if (j == i || rows[j].skipped || rows[j].algorithm == ALGOS[5].name)
                    continue;
                if (rows[j].res.wall_ms < rows[i].res.wall_ms)
                    rank++;
            }
            rows[i].rank = rank;
        }
    }
    return count;
}

void print_table(const Row *rows, int nrows)
{
    printf("%-20s | %10s | %-10s | %10s | %9s | %12s | %4s\n",
           "Algorithm", "Array size", "Type", "Time (ms)", "CPU (ms)", "Memory (MB)", "Rank");
    printf("---------------------+------------+------------+------------+-----------+--------------+-----\n");
    for (int i = 0; i < nrows; i++) {
        const Row *r = &rows[i];
        if (r->skipped) {
            printf("%-20s | %10d | %-10s | %10s | %9s | %12s | %4s\n",
                   r->algorithm, r->size, KIND_NAMES[r->kind], "skipped", "-", "-", "-");
        } else {
            char rank[16] = "-";
            if (r->rank > 0)
                snprintf(rank, sizeof rank, "%d", r->rank);
            printf("%-20s | %10d | %-10s | %10.3f | %9.3f | %12.3f | %4s\n",
                   r->algorithm, r->size, KIND_NAMES[r->kind],
                   r->res.wall_ms, r->res.cpu_ms, r->res.mem_kb / 1024.0, rank);
        }
    }
}

int save_csv(const Row *rows, int nrows, const char *path)
{
    if (mkdir("results", 0755) != 0 && errno != EEXIST)
        return -1;
    FILE *f = fopen(path, "w");
    if (!f)
        return -1;
    fprintf(f, "algorithm,size,kind,wall_ms,cpu_ms,mem_mb,rank\n");
    for (int i = 0; i < nrows; i++) {
        const Row *r = &rows[i];
        if (r->skipped)
            fprintf(f, "%s,%d,%s,,,,\n", r->algorithm, r->size, KIND_NAMES[r->kind]);
        else
            fprintf(f, "%s,%d,%s,%.4f,%.4f,%.4f,%d\n", r->algorithm, r->size,
                    KIND_NAMES[r->kind], r->res.wall_ms, r->res.cpu_ms,
                    r->res.mem_kb / 1024.0, r->rank);
    }
    fclose(f);
    return 0;
}
