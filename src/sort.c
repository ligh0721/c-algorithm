#include <stdio.h>
#include "sort.h"


extern void swap(Object* a, Object* b);

void print_int_array(const Object arr[], int size) {
    for (int i=0; i<size; ++i) {
        printf("%ld ", arr[i].int_value);
    }
    printf("\n");
}

void bubble_sort(Object arr[], int size, compare_function compare) {
    for (int i=size-1; i>=1; --i) {
        for (int j=1; j<=i; ++j) {
            if (!compare(arr[j-1], arr[j])) {
                swap(&arr[j-1], &arr[j]);
            }
        }
    }
}

void insert_sort(Object arr[], int size, compare_function compare) {
    for (int i=1; i<size; ++i) {
        Object k = arr[i];
        int j = i;
        for (; j>0 && !compare(arr[j-1], k); --j) {
            arr[j] = arr[j-1];
        }
        if (j != i) {
            arr[j] = k;
        }
    }
}

void _quick_sort(Object arr[], int l, int r, compare_function compare) {
    Object k = arr[l];
    int i = l;
    int j = r;
    while (i < j) {
        for (; j>i && compare(k, arr[j]); --j);
        if (j > i) {
            arr[i] = arr[j];
            ++i;
        }
        for (; i<j && compare(arr[i], k); ++i);
        if (i < j) {
            arr[j] = arr[i];
            --j;
        }
    }
    arr[i] = k;
    if (l < i-1) {
        _quick_sort(arr, l, i-1, compare);
    }
    if (i+1 < r) {
        _quick_sort(arr, i+1, r, compare);
    }
}

void quick_sort(Object arr[], int size, compare_function compare) {
    if (size > 1) {
        _quick_sort(arr, 0, size-1, compare);
    }
}

void sort(Object arr[], int size, compare_function compare) {
    if (size > 40) {
        quick_sort(arr, size, compare);
    } else {
        insert_sort(arr, size, compare);
    }
}
