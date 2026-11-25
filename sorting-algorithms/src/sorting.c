#include "sorting.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void swap(int *x, int *y)
{
    int t = *x;
    *x = *y;
    *y = t;
}

/* Bubble Sort with early exit when a full pass makes no swap. */
void bubble_sort(int *a, int n)
{
    for (int i = 0; i < n - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < n - 1 - i; j++) {
            if (a[j] > a[j + 1]) {
                swap(&a[j], &a[j + 1]);
                swapped = 1;
            }
        }
        if (!swapped)
            break;
    }
}

/* Selection Sort: swap the minimum of the unsorted part into place. */
void selection_sort(int *a, int n)
{
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++) {
            if (a[j] < a[min_idx])
                min_idx = j;
        }
        if (min_idx != i)
            swap(&a[i], &a[min_idx]);
    }
}

/* Insertion Sort: insert each element into the sorted left part. */
void insertion_sort(int *a, int n)
{
    for (int i = 1; i < n; i++) {
        int key = a[i];
        int j = i - 1;
        while (j >= 0 && a[j] > key) {
            a[j + 1] = a[j];
            j--;
        }
        a[j + 1] = key;
    }
}

/* Merge Sort: top-down, one auxiliary buffer (O(n) extra space).
 * Recursion depth is log2(n): safe for large arrays. Sorts a[lo..hi-1]. */
static void merge_rec(int *a, int *tmp, int lo, int hi)
{
    if (hi - lo < 2)
        return;
    int mid = lo + (hi - lo) / 2;
    merge_rec(a, tmp, lo, mid);
    merge_rec(a, tmp, mid, hi);

    int i = lo, j = mid, k = lo;
    while (i < mid && j < hi)
        tmp[k++] = (a[i] <= a[j]) ? a[i++] : a[j++]; /* <= keeps it stable */
    while (i < mid)
        tmp[k++] = a[i++];
    while (j < hi)
        tmp[k++] = a[j++];
    memcpy(a + lo, tmp + lo, (size_t)(hi - lo) * sizeof(int));
}

void merge_sort(int *a, int n)
{
    if (n < 2)
        return;
    int *tmp = malloc((size_t)n * sizeof(int));
    if (!tmp) {
        fprintf(stderr, "merge_sort: out of memory\n");
        exit(1);
    }
    merge_rec(a, tmp, 0, n);
    free(tmp);
}

/* Quick Sort.
 * Pivot strategy: median-of-three (first, middle, last values).
 * Partition: 3-way (Dutch national flag) -> efficient with many duplicates.
 * Recursion only on the smaller part, loop on the larger one, so the
 * recursion depth stays <= log2(n) (no stack overflow, even on sorted data). */
static int median_of_three(int x, int y, int z)
{
    if (x < y) {
        if (y < z) return y;
        return (x < z) ? z : x;
    }
    if (x < z) return x;
    return (y < z) ? z : y;
}

static void quick_rec(int *a, int lo, int hi)
{
    while (lo < hi) {
        int pivot = median_of_three(a[lo], a[lo + (hi - lo) / 2], a[hi]);

        /* [lo..lt-1] < pivot, [lt..gt] == pivot, [gt+1..hi] > pivot */
        int lt = lo, i = lo, gt = hi;
        while (i <= gt) {
            if (a[i] < pivot) {
                swap(&a[lt], &a[i]);
                lt++;
                i++;
            } else if (a[i] > pivot) {
                swap(&a[i], &a[gt]);
                gt--;
            } else {
                i++;
            }
        }

        if (lt - lo < hi - gt) {
            quick_rec(a, lo, lt - 1);
            lo = gt + 1;
        } else {
            quick_rec(a, gt + 1, hi);
            hi = lt - 1;
        }
    }
}

void quick_sort(int *a, int n)
{
    quick_rec(a, 0, n - 1);
}
