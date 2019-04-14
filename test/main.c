//
// Created by t5w0rd on 19-4-13.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sort.h"
#include "stack.h"
#include "array.h"


typedef struct {
    int key;
    char msg[256];
} ITEM;

VALUE new_item(int key, const char* msg) {
    ITEM* obj = (ITEM*)malloc(sizeof(ITEM));
    obj->key = key;
    strcpy(obj->msg, msg);
    VALUE ret;
    ret.ptr_value = obj;
    return ret;
}

void print_item(const VALUE value) {
    ITEM* v = (ITEM*)value.ptr_value;
    printf("{%d: %s} ", v->key, v->msg);
}

void print_array_item(const VALUE arr[], size_t size) {
    for (size_t i=0; i<size; ++i) {
        print_item(arr[i]);
    }
    printf("\n");
}

int asc_order_item(const VALUE a, const VALUE b) {
    ITEM* ela = (ITEM*)a.ptr_value;
    ITEM* elb = (ITEM*)b.ptr_value;
    return ela->key <= elb->key;
}

int desc_order_item(const VALUE a, const VALUE b) {
    ITEM* ela = (ITEM*)a.ptr_value;
    ITEM* elb = (ITEM*)b.ptr_value;
    return ela->key >= elb->key;
}

void sort_test() {
    printf("=== sort test ===\n");
    VALUE arr[] = {
            new_item(6, "six"),
            new_item(8, "eight"),
            new_item(1, "one"),
            new_item(2, "two"),
            new_item(-3, "-three"),
            new_item(4, "four"),
            new_item(4, "four2"),
            new_item(9, "nine"),
            new_item(5, "five"),
            new_item(6, "six2"),
            new_item(6, "six3")
    };
    size_t size = sizeof(arr)/sizeof(arr[0]);
    print_array_item(arr, size);
    merge_sort(arr, size, desc_order_item);
    print_array_item(arr, size);
    printf("\n");
}

void stack_test() {
    printf("=== stack test ===\n");
    STACK* st = open_stack();
    stack_push(st, new_item(9, "nine"));
    stack_push(st, new_item(8, "eight"));
    stack_push(st, new_item(7, "seven"));
    stack_push(st, new_item(6, "six"));
    for (VALUE i=stack_pop(st); !is_null_object(i); i=stack_pop(st)) {
        print_item(i);
    }
    printf("\n");
    close_stack(st);
    printf("\n");
}

void array_test() {
    printf("=== array test ===\n");
    VALUE org_arr[] = {
            new_item(0, "zero"),
            new_item(1, "one"),
            new_item(2, "two"),
            new_item(3, "three"),
            new_item(4, "four"),
            new_item(5, "five"),
    };
    size_t size = sizeof(org_arr)/sizeof(org_arr[0]);

    ARRAY* arr = open_array_by_data(org_arr, size);
    SLICE* sl1 = open_slice_by_array(arr, 1, 4);
    SLICE* sl2 = open_slice_by_slice(sl1, 2, 5);
    SLICE* sl3 = open_slice(0, 5);

    printf("arr len: %zu cap: %zu\n", array_len(arr), array_cap(arr));
    printf("sl1 len: %zu cap: %zu\n", slice_len(sl1), slice_cap(sl1));
    printf("sl2 len: %zu cap: %zu\n", slice_len(sl2), slice_cap(sl2));

    slice_append(sl2, new_item(6, "six"));

    slice_set(sl2, 0, new_item(-3, "-three"));

    for (int i=0; i<slice_len(sl1); ++i) {
        print_item(slice_get(sl1, i));
    }
    printf("\n");
    for (int i=0; i<slice_len(sl2); ++i) {
        print_item(slice_get(sl2, i));
    }
    printf("\n");
    printf("sl3 len: %zu cap: %zu\n", slice_len(sl3), slice_cap(sl3));

    close_slice(sl1);
    close_slice(sl2);
    close_slice(sl3);
    close_array(arr);
    printf("\n");
}

int main(int argc, char* argv[]) {
    sort_test();
    stack_test();
    array_test();

    return 0;
}
