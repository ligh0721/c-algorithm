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

//extern struct linked_node* open_link_node(VALUE value, struct linked_node *next);

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

void stack_push(STACK* st, VALUE value) {
    assert(st != NULL);
    slice_append(st->data, value);
}

VALUE stack_pop(STACK* st) {
    assert(st != NULL);
    long len = slice_len(st->data);
    if (len == 0) {
        return NULL_VALUE;
    }
    return slice_remove(st->data, slice_len(st->data)-1);
}

