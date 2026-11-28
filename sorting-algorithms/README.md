# Sorting Algorithms in C

Assignment: Algorithms and Data Structures, Badji Mokhtar University.
Name: Assaad Chemseddine Kherfane
DD2

This project has 5 sorting algorithms written in C: bubble sort, selection sort, insertion sort, merge sort and quick sort. I measure the time and the memory of each one and compare them.

## How to run

You need gcc and make.

    make
    ./build/sorting

Then choose an option in the menu. To run the tests use `make test`. To run all the experiments use `make experiments`.

## Files

- src/sorting.c : the 5 algorithms
- src/benchmark.c : time and memory
- src/utils.c : makes the arrays
- src/main.c : the menu
- tests/ : tests
- results/ : results in csv and charts

## Measuring

- Time: clock_gettime and clock()
- Memory: getrusage
- Same array for all algorithms (fixed seed 42), average of several runs
- No built in sort used in the algorithms

## Results (random array, time in ms)

| Size | Bubble | Selection | Insertion | Merge | Quick |
|---|---|---|---|---|---|
| 1,000 | 1.87 | 1.27 | 0.11 | 0.06 | 0.05 |
| 10,000 | 182 | 126 | 13.4 | 0.69 | 0.89 |
| 50,000 | 6866 | 3196 | 206 | 5.3 | 4.8 |
| 100,000 | skipped | skipped | skipped | 11.2 | 9.7 |

Bubble, selection and insertion are skipped at 100,000 because they take too long. The other tables and charts are in the results folder.

Merge sort uses about 0.5 MB of extra memory at 100,000. The others use almost nothing.

## Conclusion

Merge sort and quick sort are much faster than the other three on big arrays. Bubble sort and insertion sort are very fast when the array is already sorted. Selection sort is always slow. Merge sort needs extra memory, quick sort does not.
