//
// Created by t5w0rd on 19-4-14.
//

#include <stdlib.h>
#include <assert.h>
#include "algorithm.h"
#include "link.h"


struct list {
    struct linked_node* head;
    struct linked_node* tail;
};

struct linked_node* open_link_node(VALUE value, struct linked_node* prev, struct linked_node* next) {
    struct linked_node* ret = NEW(struct linked_node);
    ret->value = value;
    ret->next = next;
    ret->prev = prev;
    return ret;
}

void close_link_node(struct linked_node* node) {
    if (node->next != NULL) {
        node->next->prev = node->prev;
    }
    if (node->prev != NULL) {
        node->prev->next = node->next;
    }
    DELETE(node);
}

LIST* open_list() {
    struct list* ret = NEW(struct list);
    assert(ret != NULL);
    ret->tail = ret->head = NULL;
    return ret;
}

void close_list(LIST* lst) {
    assert(lst != NULL);
    struct linked_node* next;
    for (struct linked_node* p=lst->head; p!=NULL; p=next) {
        next = p->next;
        DELETE(p);
    }
}
