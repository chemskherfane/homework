#ifndef SORTING_H
#define SORTING_H

/* The five sorting algorithms. Each sorts a[0..n-1] IN PLACE, ascending.
 * No built-in sort is used here, and no menu/display/measurement code. */

void bubble_sort(int *a, int n);
void selection_sort(int *a, int n);
void insertion_sort(int *a, int n);
void merge_sort(int *a, int n);
void quick_sort(int *a, int n);

#endif
