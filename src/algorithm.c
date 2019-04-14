//
// Created by t5w0rd on 19-4-13.
//

#include <stdio.h>
#include "algorithm.h"


int asc_order_int(const Object a, const Object b) {
    return a.int_value <= b.int_value;
}

int desc_order_int(const Object a, const Object b) {
    return a.int_value >= b.int_value;
}

void print_array_int(const Object *arr, int size) {
    for (int i=0; i<size; ++i) {
        printf("%ld ", arr[i].int_value);
    }
    printf("\n");
}
