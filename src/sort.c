//
// Created by t5w0rd on 19-4-13.
//

#include <stdlib.h>
#include "sort.h"


extern void swap(VALUE* a, VALUE* b);

void bubble_sort(VALUE arr[], size_t size, compare_function compare) {
    for (size_t i=size-1; i>=1; --i) {
        for (size_t j=1; j<=i; ++j) {
            if (!compare(arr[j-1], arr[j])) {
                swap(&arr[j-1], &arr[j]);
            }
        }
    }
}

void select_sort(VALUE arr[], size_t size, compare_function compare) {
    for (size_t i=1; i<size; ++i) {
        size_t k = i-1;
        for (size_t j=i; j<size; ++j) {
            if (!compare(arr[k], arr[j])) {
                k = j;
            }
        }
        if (k != i-1) {
            swap(&arr[k], &arr[i-1]);
        }
    }
}

void insert_sort(VALUE arr[], size_t size, compare_function compare) {
    for (int i=1; i<size; ++i) {
        VALUE k = arr[i];
        size_t j = i;
        for (; j>0 && !compare(arr[j-1], k); --j) {
            arr[j] = arr[j-1];
        }
        if (j != i) {
            arr[j] = k;
        }
    }
}

void _quick_sort(VALUE arr[], size_t l, size_t r, compare_function compare) {
    VALUE k = arr[l];
    size_t i = l;
    size_t j = r;
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

void quick_sort(VALUE arr[], size_t size, compare_function compare) {
    if (size > 1) {
        _quick_sort(arr, 0, size-1, compare);
    }
}

void merge_sort(VALUE arr[], size_t size, compare_function compare) {
    VALUE* t = (VALUE*)malloc(sizeof(VALUE)*size);
    for (size_t i=1; i<size; i+=i) {
        size_t l, ml, r, mr;
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

void sort(VALUE arr[], size_t size, compare_function compare) {
    if (size > 60) {
        merge_sort(arr, size, compare);
    } else {
        insert_sort(arr, size, compare);
    }
}
