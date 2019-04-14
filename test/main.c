//
// Created by t5w0rd on 19-4-13.
//
#include <stdio.h>
#include "sort.h"


int main(int argc, char* argv[]) {
    Object arr[] = {{6}, {8}, {1}, {2}, {-3}, {4}, {9}, {5}};
    int size = sizeof(arr)/sizeof(arr[0]);
    print_int_array(arr, size);
    sort(arr, size, int_asc_order);
    print_int_array(arr, size);
    return 0;
}