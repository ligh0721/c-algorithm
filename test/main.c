//
// Created by t5w0rd on 19-4-13.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sort.h"


typedef struct {
    int key;
    char msg[256];
} Item;

Object new_item(int key, const char* msg) {
    Item* obj = (Item*)malloc(sizeof(Item));
    obj->key = key;
    strcpy(obj->msg, msg);
    Object ret;
    ret.pointer_value = obj;
    return ret;
}

void print_array_item(const Object *arr, int size) {
    for (int i=0; i<size; ++i) {
        Item* el = (Item*)arr[i].pointer_value;
        printf("{%d: %s} ", el->key, el->msg);
    }
    printf("\n");
}

int asc_order_item(const Object a, const Object b) {
    Item* ela = (Item*)a.pointer_value;
    Item* elb = (Item*)b.pointer_value;
    return ela->key <= elb->key;
}

int desc_order_item(const Object a, const Object b) {
    Item* ela = (Item*)a.pointer_value;
    Item* elb = (Item*)b.pointer_value;
    return ela->key >= elb->key;
}

int main(int argc, char* argv[]) {
    Object arr[] = {
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
    int size = sizeof(arr)/sizeof(arr[0]);
    print_array_item(arr, size);
    merge_sort(arr, size, desc_order_item);
    print_array_item(arr, size);
    return 0;
}