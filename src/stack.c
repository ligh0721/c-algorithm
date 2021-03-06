//
// Created by t5w0rd on 19-4-14.
//

#include <stdlib.h>
#include <assert.h>
#include "array.h"
#include "stack.h"


struct stack {
    SLICE* data;
};

//extern struct dlnode* open_link_node(VALUE value, struct dlnode *next);

STACK* open_stack(long cap) {
    struct stack* ret = NEW(struct stack);
    ret->data = open_slice(0, cap);
    return ret;
}

void close_stack(STACK* st) {
    assert(st != NULL);
    close_slice(st->data);
    DELETE(st);
}

long stack_len(STACK* st) {
    assert(st != NULL);
    return slice_len(st->data);
}

void stack_push(STACK* st, VALUE value) {
    assert(st != NULL);
    slice_append(st->data, value);
}

VALUE stack_pop(STACK* st, int* empty) {
    assert(st != NULL);
    long len = slice_len(st->data);
    if (len == 0) {
        if (empty != NULL) {
            *empty = 1;
        }
        return EMPTY_VALUE;
    }
    return slice_pop(st->data, slice_len(st->data) - 1);
}

VALUE stack_top(STACK* st) {
    assert(st != NULL);
    long len = slice_len(st->data);
    if (len == 0) {
        return EMPTY_VALUE;
    }
    return slice_get(st->data, len-1);
}

SLICE* stack_data(STACK* st) {
    assert(st != NULL);
    return st->data;
}