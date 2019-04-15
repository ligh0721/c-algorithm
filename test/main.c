//
// Created by t5w0rd on 19-4-13.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sort.h"
#include "stack.h"
#include "array.h"
#include "heap.h"
#include "deque.h"
#include "main.h"


typedef struct {
    int key;
    char msg[256];
} ITEM;

VALUE new_item(int key, const char* msg) {
    ITEM* obj = NEW(ITEM);
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

void print_array_item(const VALUE arr[], long size) {
    for (long i=0; i<size; ++i) {
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
    long size = sizeof(arr)/sizeof(arr[0]);
    print_array_item(arr, size);
    merge_sort(arr, size, desc_order_item);
    print_array_item(arr, size);
    printf("\n");
}

void stack_test() {
    printf("=== stack test ===\n");
    STACK* st = open_stack(0);
    stack_push(st, new_item(9, "nine"));
    stack_push(st, new_item(8, "eight"));
    stack_push(st, new_item(7, "seven"));
    stack_push(st, new_item(6, "six"));
    for (VALUE i=stack_pop(st); !is_null_value(i); i=stack_pop(st)) {
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
    long size = sizeof(org_arr)/sizeof(org_arr[0]);

    ARRAY* arr = open_array_by_data(org_arr, size);
    SLICE* sl1 = open_slice_by_array(arr, 1, 4);
    SLICE* sl2 = open_slice_by_slice(sl1, 2, 5);


    printf("arr len: %ld cap: %ld\n", array_len(arr), array_cap(arr));
    printf("sl1 len: %ld cap: %ld\n", slice_len(sl1), slice_cap(sl1));
    printf("sl2 len: %ld cap: %ld\n", slice_len(sl2), slice_cap(sl2));


    slice_append(sl2, new_item(6, "six"));
    slice_pop(sl2, slice_len(sl2) - 1);
//    slice_pop(sl2, 0);

    slice_set(sl2, 0, new_item(-3, "-three"));
//    merge_sort(slice_data(sl2), slice_len(sl2), desc_order_item);

    for (int i=0; i<slice_len(sl1); ++i) {
        print_item(slice_get(sl1, i));
    }
    printf("\n");
    for (int i=0; i<slice_len(sl2); ++i) {
        print_item(slice_get(sl2, i));
    }
    printf("\n");


    SLICE* sl3 = open_slice(0, 0);
    slice_append(sl3, int_value(0));
    slice_append(sl3, int_value(1));
    slice_append(sl3, int_value(2));
    slice_append(sl3, int_value(3));
    slice_append(sl3, int_value(4));
    slice_append(sl3, int_value(5));
    printf("sl3 len: %ld cap: %ld\n", slice_len(sl3), slice_cap(sl3));

    SLICE* sl4 = open_slice_by_slice(sl3, 3, 6);
    slice_pop(sl4, 0);

    for (int i=0; i<slice_len(sl4); ++i) {
        printf("%ld ", slice_get(sl4, i).int_value);
    }
    printf("\n");

    close_slice(sl1);
    close_slice(sl2);
    close_slice(sl3);
    close_slice(sl4);
    close_array(arr);
    printf("\n");
}

void heap_test() {
    printf("=== heap test ===\n");
    HEAP* hp = open_heap(asc_order_item, 3);
    heap_push(hp, new_item(5, "five"));
    heap_push(hp, new_item(8, "eight"));
    heap_push(hp, new_item(3, "three"));
    heap_push(hp, new_item(1, "one"));
    heap_push(hp, new_item(6, "six"));
    heap_push(hp, new_item(9, "nine"));
    heap_push(hp, new_item(2, "two"));
    heap_push(hp, new_item(1, "one2"));

    heap_pop(hp);
    heap_pop(hp);
    heap_pop(hp);

    SLICE* sl = _heap_data(hp);
    for (long i=0; i<slice_len(sl); ++i) {
        print_item(slice_get(sl, i));
    }
    printf("\n");
    close_heap(hp);


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
    long size = sizeof(arr)/sizeof(arr[0]);
    hp = open_heap_by_data(asc_order_item, arr, size);
    sl = _heap_data(hp);
    for (long i=0; i<slice_len(sl); ++i) {
        print_item(slice_get(sl, i));
    }
    printf("\n");
    close_heap(hp);
    printf("\n");
}

void deque_test() {
    printf("=== deque test ===\n");
    DEQUE* q = open_deque(5);
    deque_push_back(q, int_value(0));
    deque_push_back(q, int_value(1));
    deque_push_back(q, int_value(2));
    deque_push_back(q, int_value(3));

    deque_push_front(q, int_value(-1));
    deque_push_front(q, int_value(-2));
    deque_push_front(q, int_value(-3));

    ARRAY* arr = _deque_data(q);
    for (int i=0; i<array_cap(arr); ++i) {
        printf("%ld ", array_get(arr, i).int_value);
    }
    printf("\n");

    close_deque(q);
    printf("\n");
}

void test() {
    printf("=== test ===\n");

    printf("\n");
}

int main(int argc, char* argv[]) {
    sort_test();
    stack_test();
    array_test();
    heap_test();
    deque_test();
    test();

    return 0;
}
