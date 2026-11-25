/* Interactive menu.  Build with `make`, run with `./build/sorting`.
 * `./build/sorting --experiments` runs every size and input type
 * non-interactively and writes the CSV files into results/. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "benchmark.h"
#include "sorting.h"
#include "utils.h"

#define BIG_SIZE 100000

static const int SIZES[] = {1000, 5000, 10000, 50000, 100000};
#define NUM_SIZES ((int)(sizeof SIZES / sizeof SIZES[0]))

/* Read an integer in [low, high]. Returns 0 on end of input (Ctrl+D). */
static int ask_int(const char *prompt, int low, int high, int *out)
{
    char line[64];
    for (;;) {
        printf("%s", prompt);
        fflush(stdout);
        if (!fgets(line, sizeof line, stdin))
            return 0;
        char *end;
        long v = strtol(line, &end, 10);
        while (*end == ' ' || *end == '\t')
            end++;
        if (end == line || (*end != '\n' && *end != '\0')) {
            printf("Invalid input: please type a number.\n");
            continue;
        }
        if (v < low || v > high) {
            printf("Please choose a number between %d and %d.\n", low, high);
            continue;
        }
        *out = (int)v;
        return 1;
    }
}

static int choose_kind(Kind *kind)
{
    printf("Input type:\n");
    for (int i = 0; i < NUM_KINDS; i++)
        printf("  %d. %s\n", i + 1, KIND_NAMES[i]);
    int c;
    if (!ask_int("Your choice: ", 1, NUM_KINDS, &c))
        return 0;
    *kind = (Kind)(c - 1);
    return 1;
}

static int choose_size(int *size)
{
    printf("Array size:\n");
    for (int i = 0; i < NUM_SIZES; i++)
        printf("  %d. %d\n", i + 1, SIZES[i]);
    printf("  %d. Custom\n", NUM_SIZES + 1);
    int c;
    if (!ask_int("Your choice: ", 1, NUM_SIZES + 1, &c))
        return 0;
    if (c <= NUM_SIZES) {
        *size = SIZES[c - 1];
        return 1;
    }
    return ask_int("Size (1 - 1000000): ", 1, 1000000, size);
}

static int run_single(const Algo *algo)
{
    int n;
    Kind kind;
    if (!choose_size(&n) || !choose_kind(&kind))
        return 0;

    if (algo->quadratic && n > QUADRATIC_LIMIT) {
        printf("Warning: %s is O(n^2); %d elements may take a long time.\n", algo->name, n);
        char answer[16];
        printf("Continue? (y/n): ");
        fflush(stdout);
        if (!fgets(answer, sizeof answer, stdin))
            return 0;
        if (answer[0] != 'y' && answer[0] != 'Y')
            return 1;
    }

    int *data = generate_array(n, kind, SEED);
    if (n <= 20)
        print_array("Before:", data, n);
    printf("Running %s on %d elements (%s)...\n", algo->name, n, KIND_NAMES[kind]);
    Result r = measure(algo->fn, data, n, kind, default_runs(n));
    if (n <= 20) {
        algo->fn(data, n);
        print_array("After: ", data, n);
    }
    printf("Time (wall) : %.3f ms\n", r.wall_ms);
    printf("Time (CPU)  : %.3f ms\n", r.cpu_ms);
    printf("Memory peak : %.3f MB (extra memory used by the sort)\n", r.mem_kb / 1024.0);
    free(data);
    return 1;
}

static void run_compare_kind(Kind kind)
{
    Row rows[NUM_SIZES * NUM_ALGOS];
    printf("Sizes: 1000, 5000, 10000, 50000, 100000 (O(n^2) skipped above %d)\n", QUADRATIC_LIMIT);
    int count = compare(SIZES, NUM_SIZES, kind, 1, rows);
    printf("\n");
    print_table(rows, count);

    char path[64];
    snprintf(path, sizeof path, "results/comparison_%s.csv", KIND_NAMES[kind]);
    if (save_csv(rows, count, path) == 0)
        printf("\nResults saved to %s\n", path);
    else
        printf("\nCould not write %s\n", path);
}

static int run_compare(void)
{
    Kind kind;
    if (!choose_kind(&kind))
        return 0;
    run_compare_kind(kind);
    return 1;
}

static void run_big_example(void)
{
    Row rows[NUM_ALGOS];
    int size = BIG_SIZE;
    printf("Big array: %d random integers, seed=%u\n", BIG_SIZE, SEED);
    int count = compare(&size, 1, KIND_RANDOM, 1, rows);
    printf("\n");
    print_table(rows, count);
    printf("\nO(n^2) algorithms are skipped above %d elements (see README); "
           "they are measured up to that size in option 6.\n", QUADRATIC_LIMIT);
    save_csv(rows, count, "results/big_array.csv");
}

static void run_experiments(void)
{
    for (int k = 0; k < NUM_KINDS; k++) {
        printf("\n=== %s data ===\n", KIND_NAMES[k]);
        run_compare_kind((Kind)k);
    }
}

int main(int argc, char **argv)
{
    benchmark_self_path = argv[0];
    if (argc == 5 && strcmp(argv[1], "--measure-memory") == 0)
        return memory_child_main(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
    if (argc > 1 && strcmp(argv[1], "--experiments") == 0) {
        run_experiments();
        return 0;
    }

    for (;;) {
        printf("\n=========== SORTING ALGORITHMS ===========\n"
               "1. Bubble Sort\n"
               "2. Selection Sort\n"
               "3. Insertion Sort\n"
               "4. Merge Sort\n"
               "5. Quick Sort\n"
               "6. Compare all algorithms\n"
               "7. Big array example (100000 integers)\n"
               "0. Exit\n\n");
        int choice;
        if (!ask_int("Your choice: ", 0, 7, &choice))
            break; /* end of input */
        if (choice == 0)
            break;

        int ok = 1;
        if (choice >= 1 && choice <= 5)
            ok = run_single(&ALGOS[choice - 1]);
        else if (choice == 6)
            ok = run_compare();
        else
            run_big_example();
        if (!ok)
            break;
    }
    printf("Goodbye!\n");
    return 0;
}
