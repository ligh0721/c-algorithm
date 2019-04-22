//
// Created by t5w0rd on 19-4-13.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "sort.h"
#include "stack.h"
#include "array.h"
#include "heap.h"
#include "deque.h"
#include "rbtree.h"
#include "skiplist.h"
#include "wstring.h"
#include "link.h"
#include "main.h"


typedef struct ITEM {
    int key;
    char msg[256];
} ITEM;

struct item_node {
    ITEM* item;
    struct item_node* next;
};

static LLIST* g_item_list;

static int close_every_item(VALUE value, void* param) {
    DELETE(value.ptr_value);
    return 0;
}

VALUE open_item(int key, const char *msg) {
    ITEM* obj = NEW(ITEM);
    obj->key = key;
    strcpy(obj->msg, msg);
    VALUE ret;
    ret.ptr_value = obj;

    llist_push_back(g_item_list, ret);
    return ret;
}

void print_item(VALUE value) {
    ITEM* v = (ITEM*)value.ptr_value;
    printf("{%d: %s} ", v->key, v->msg);
}

void print_array_item(VALUE arr[], long size) {
    for (long i=0; i<size; ++i) {
        print_item(arr[i]);
    }
    printf("\n");
}

int asc_order_item(VALUE a, VALUE b) {
    ITEM* ela = (ITEM*)a.ptr_value;
    ITEM* elb = (ITEM*)b.ptr_value;
    return ela->key - elb->key;
}

int asc_order_item2(VALUE a, VALUE b) {
    ITEM* ela = (ITEM*)a.ptr_value;
    ITEM* elb = (ITEM*)b.ptr_value;
    return strcmp(ela->msg, elb->msg);
}

int desc_order_item(VALUE a, VALUE b) {
    ITEM* ela = (ITEM*)a.ptr_value;
    ITEM* elb = (ITEM*)b.ptr_value;
    return elb->key - ela->key;
}

void save_data() {
    int len = 0xfffff;
    int* data = (int*)malloc(sizeof(int)*len);
    for (int i=0; i<len; ++i) {
        data[i] = rand();
    }

    int* data2 = (int*)malloc(sizeof(int)*len);
    for (int i=0; i<len; ++i) {
        data2[i] = data[rand()%len];
    }
    FILE* fp = fopen("data.dat", "wb");
    fwrite(&len, sizeof(len), 1, fp);
    fwrite(data, sizeof(data[0]), len, fp);
    fwrite(&len, sizeof(len), 1, fp);
    fwrite(data2, sizeof(data2[0]), len, fp);
    fclose(fp);
    free(data);
    free(data2);
}

void load_data(int** data, int** data2, int* len) {
    FILE* fp = fopen("data.dat", "rb");
    fread(len, sizeof(int), 1, fp);
    *data = (int*)malloc(sizeof(int)*(*len));
    fread(*data, sizeof(int), *len, fp);
    fread(len, sizeof(int), 1, fp);
    *data2 = (int*)malloc(sizeof(int)*(*len));
    fread(*data2, sizeof(int), *len, fp);
    fclose(fp);
}

void sort_test() {
    printf("=== sort test ===\n");
    VALUE arr[] = {
            open_item(6, "six"),
            open_item(8, "eight"),
            open_item(1, "one"),
            open_item(2, "two"),
            open_item(-3, "-three"),
            open_item(4, "four"),
            open_item(4, "four2"),
            open_item(9, "nine"),
            open_item(5, "five"),
            open_item(6, "six2"),
            open_item(6, "six3")
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
    stack_push(st, open_item(9, "nine"));
    stack_push(st, open_item(8, "eight"));
    stack_push(st, open_item(7, "seven"));
    stack_push(st, open_item(6, "six"));
    int empty = 0;
    for (VALUE i=stack_pop(st, &empty); !empty; i=stack_pop(st, &empty)) {
        print_item(i);
    }
    printf("\n");
    close_stack(st);
    printf("\n");
}

void array_test() {
    printf("=== array test ===\n");
    VALUE org_arr[] = {
            open_item(0, "zero"),
            open_item(1, "one"),
            open_item(2, "two"),
            open_item(3, "three"),
            open_item(4, "four"),
            open_item(5, "five"),
    };
    long size = sizeof(org_arr)/sizeof(org_arr[0]);

    ARRAY* arr = open_array_by_data(org_arr, size);
    SLICE* sl1 = open_slice_by_array(arr, 1, 4);
    SLICE* sl2 = open_slice_by_slice(sl1, 2, 5);


    printf("arr len: %ld cap: %ld\n", array_len(arr), array_cap(arr));
    printf("sl1 len: %ld cap: %ld\n", slice_len(sl1), slice_cap(sl1));
    printf("sl2 len: %ld cap: %ld\n", slice_len(sl2), slice_cap(sl2));


    slice_append(sl2, open_item(6, "six"));
    slice_pop(sl2, slice_len(sl2) - 1);
//    slice_pop(sl2, 0);

    slice_set(sl2, 0, open_item(-3, "-three"));
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
    heap_push(hp, open_item(5, "five"));
    heap_push(hp, open_item(8, "eight"));
    heap_push(hp, open_item(3, "three"));
    heap_push(hp, open_item(1, "one"));
    heap_push(hp, open_item(6, "six"));
    heap_push(hp, open_item(9, "nine"));
    heap_push(hp, open_item(2, "two"));
    heap_push(hp, open_item(1, "one2"));

    heap_pop(hp, NULL);
    heap_pop(hp, NULL);
    heap_pop(hp, NULL);

    SLICE* sl = _heap_data(hp);
    for (long i=0; i<slice_len(sl); ++i) {
        print_item(slice_get(sl, i));
    }
    printf("\n");
    close_heap(hp);


    VALUE arr[] = {
            open_item(6, "six"),
            open_item(8, "eight"),
            open_item(1, "one"),
            open_item(2, "two"),
            open_item(-3, "-three"),
            open_item(4, "four"),
            open_item(4, "four2"),
            open_item(9, "nine"),
            open_item(5, "five"),
            open_item(6, "six2"),
            open_item(6, "six3")
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


    int empty = 0;
    for (VALUE i=deque_pop_back(q, &empty); !empty; i=deque_pop_back(q, &empty)) {
        printf("%ld ", i.int_value);
    }
    printf("\n");

    deque_push_back(q, int_value(88));
    deque_push_front(q, int_value(-88));

    empty = 0;
    for (VALUE i=deque_pop_front(q, &empty); !empty; i=deque_pop_front(q, &empty)) {
        printf("%ld ", i.int_value);
    }
    printf("\n");


//    ARRAY* arr = _deque_data(q);
//    for (int i=0; i<array_cap(arr); ++i) {
//        printf("%ld ", array_get(arr, i).int_value);
//    }
//    printf("_deque_data\n");

    close_deque(q);
    printf("\n");
}

int traverse(VALUE value, void* param) {
    printf("%ld ", value.int_value);
    return 0;
}

void rbtree_test() {
    printf("=== rbtree test ===\n");

    RBTREE* tr = open_rbtree(asc_order_int);
//    rbtree_set(tr, int_value(5));
//    rbtree_set(tr, int_value(5));
//    rbtree_set(tr, int_value(8));
//    rbtree_set(tr, int_value(3));
//    rbtree_set(tr, int_value(1));
//    rbtree_set(tr, int_value(-6));
//    rbtree_set(tr, int_value(7));
    srand((int)time(0));
    for (int i=0; i<1024; ++i) {
        rbtree_set(tr, int_value(rand()%1000));
    }

//    int ok;
//    RBNODE* search = *rbtree_fast_get(tr, int_value(5), NULL);
//    rbtree_fast_pop(tr, search);
//
//    VALUE res = rbtree_pop(tr, int_value(1), &ok);
//    printf("%ld %d\n", res.int_value, ok);

    rbtree_ldr(tr, traverse, NULL);
    printf("\n");
    close_rbtree(tr);
    printf("\n");
}

void skiplist_test() {
    extern void skiplist_debug(SKIPLIST* sl, PRINT_VALUE print);

    printf("=== skiplist test ===\n");
    SKIPLIST* sl = open_skiplist(asc_order_item, asc_order_item2);
    skiplist_set(sl, open_item(22, "22aa"));
    skiplist_set(sl, open_item(19, "19aa"));
    skiplist_set(sl, open_item(7, "7aa"));
    skiplist_set(sl, open_item(3, "3aa"));
    skiplist_set(sl, open_item(37, "37aa"));
    skiplist_set(sl, open_item(11, "11aa"));
    skiplist_set(sl, open_item(11, "11ab"));
    skiplist_set(sl, open_item(11, "11abc"));
    skiplist_set(sl, open_item(11, "11a"));
    skiplist_set(sl, open_item(26, "26aa"));

    skiplist_debug(sl, print_item);

//    for (int i=0; i<1000; ++i) {
//        skiplist_set(sl, open_item(rand()%10000, "rand"));
//    }

    printf("%ld removed\n", skiplist_remove(sl, open_item(30, ""), open_item(1, "")));
    skiplist_debug(sl, print_item);

    skiplist_set(sl, open_item(11, "11aa"));
    skiplist_set(sl, open_item(11, "11ab"));
    skiplist_set(sl, open_item(11, "11abc"));
    skiplist_set(sl, open_item(11, "11a"));
    skiplist_set(sl, open_item(26, "26aa"));
    skiplist_debug(sl, print_item);

    SLICE* sli = open_slice(0, 100);
    skiplist_range(sl, sli, open_item(2, ""), open_item(30, ""), 5);
    for (int i=0; i<slice_len(sli); ++i) {
        print_item(slice_get(sli, i));
    }
    printf("\n");
    close_slice(sli);

    close_skiplist(sl);

    printf("\n");
}

void wstring_test() {
    printf("=== wstring test ===\n");

    WSTRING* s2 = open_wstring_with_data(L"abc");
    WSTRING* s3 = open_wstring_with_data(L"abcd");

//    printf("s3, s2: %d\n", wstring_cmp(s3, s2));
    wstring_cpy(s2, s3);

    WSTRING* ss = open_wstring_with_format(L"abc%d", 1);
    close_wstring(ss);

    close_wstring(s2);
    close_wstring(s3);
    printf("\n");
}

void test() {
    printf("=== test ===\n");
//    save_data();

    int* data;
    int* data2;
    int len;
    load_data(&data, &data2, &len);

    RBTREE* tr = open_rbtree(asc_order_int);
    SKIPLIST* sl = open_skiplist(asc_order_int, NULL);

    int start;
    int ok;

    start = clock();
    for (int i=0; i<len; ++i) {
        skiplist_set(sl, int_value(data[i]));
    }
    printf("skiplist_set, %d: %ld ticks\n", len, clock()-start);

    start = clock();
    for (int i=0; i<len; ++i) {
        skiplist_get(sl, int_value(data2[i]), &ok);
    }
    printf("skiplist_get, %d: %ld ticks\n", len, clock()-start);

    start = clock();
    for (int i=0; i<len; ++i) {
        rbtree_set(tr, int_value(data[i]));
    }
    printf("tbtree_set, %d: %ld ticks\n", len, clock()-start);

    start = clock();
    for (int i=0; i<len; ++i) {
        rbtree_get(tr, int_value(data2[i]), &ok);
    }
    printf("tbtree_get, %d: %ld ticks\n", len, clock()-start);

    close_rbtree(tr);
    close_skiplist(sl);

    printf("\n");
}

typedef int int_VALUE;

#include "algorithm_tpl.h"
#include "rbtree_tpl.h"
#include "array_tpl.h"
NULL_VALUE_DEF(int, 0)
COMPARE_DEF(int)
TRAVERSE_DEF(int)
RBTREE_DECL(int)
RBTREE_DEF(int)
ARRAY_DECL(int)
ARRAY_DEF(int)

int main(int argc, char* argv[]) {
    g_item_list = open_llist();

    sort_test();
    stack_test();
    array_test();
    heap_test();
    deque_test();
    rbtree_test();
    skiplist_test();
    wstring_test();
    int_RBTREE* tr = open_int_rbtree(NULL);
    close_int_rbtree(tr);
//    test();

    llist_traversal(g_item_list, close_every_item, NULL);
    close_llist(g_item_list);
    return 0;
}
