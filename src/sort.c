#include <stdlib.h>
#include "sort.h"


extern void swap(Object* a, Object* b);

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
            arr[i++] = arr[j];
        }
        for (; i<j && compare(arr[i], k); ++i);
        if (i < j) {
            arr[j--] = arr[i];
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

void merge_sort(Object arr[], int size, compare_function compare) {
    Object* t = (Object*)malloc(sizeof(Object)*size);
    for (int i=1; i<size; i+=i) {
        int l, ml, r, mr;
        for (l=0; l+i<size; l=mr) {
            r = ml = l + i;
            mr = r + i;
            if (mr > size) {
                mr = size;
            }
            int j = 0;
            while (l<ml && r<mr) {
                t[j++] = compare(arr[l], arr[r]) ? arr[l++] : arr[r++];
            }
            while (l < ml) {
                arr[--r] = arr[--ml];
            }
            while (j > 0) {
                arr[--r] = t[--j];
            }
        }
    }
    free(t);
}

void sort(Object arr[], int size, compare_function compare) {
    if (size > 60) {
        merge_sort(arr, size, compare);
    } else {
        insert_sort(arr, size, compare);
    }
}
