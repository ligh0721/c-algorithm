//
// Created by t5w0rd on 19-4-14.
//

#include <stdlib.h>
#include <assert.h>
#include "link.h"
#include "stack.h"


struct stack {
    struct link_node* top;
};

extern struct link_node* open_link_node(VALUE value, struct link_node *next);

STACK* open_stack() {
    struct stack* ret = NEW(struct stack);
    ret->top = NULL;
    return ret;
}

void close_stack(STACK* st) {
    assert(st != NULL);
    close_all_link_node(st->top);
    DELETE(st);
}

void stack_push(STACK* st, VALUE value) {
    assert(st != NULL);
    struct link_node* node = open_link_node(value, st->top);
    st->top = node;
}

VALUE stack_pop(STACK* st) {
    assert(st != NULL);
    if (st->top == NULL) {
        return NULL_VALUE;
    }
    struct link_node* poped = st->top;
    VALUE ret = poped->value;
    st->top = st->top->next;
    free(poped);
    return ret;
}

