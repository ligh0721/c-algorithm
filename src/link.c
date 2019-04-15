//
// Created by t5w0rd on 19-4-14.
//

#include <stdlib.h>
#include <assert.h>
#include "algorithm.h"
#include "link.h"


struct linked_node* open_linked_node(VALUE value, struct linked_node* next) {
    struct linked_node* ret = NEW(struct linked_node);
    ret->value = value;
    ret->next = next;
    return ret;
}

struct dlinked_node* open_dlinked_node(VALUE value, struct dlinked_node* prev, struct dlinked_node* next) {
    struct dlinked_node* ret = NEW(struct dlinked_node);
    ret->value = value;
    ret->next = next;
    ret->prev = prev;
    return ret;
}

void close_dlinked_node(struct dlinked_node* node) {
    if (node->next != NULL) {
        node->next->prev = node->prev;
    }
    if (node->prev != NULL) {
        node->prev->next = node->next;
    }
    DELETE(node);
}

struct dlinked_list {
    struct dlinked_node* head;
    struct dlinked_node* tail;
};

DLINKED_LIST* open_dlinked_list() {
    struct dlinked_list* ret = NEW(struct dlinked_list);
    assert(ret != NULL);
    ret->tail = ret->head = NULL;
    return ret;
}

void close_dlinked_list(DLINKED_LIST* lst) {
    assert(lst != NULL);
    struct dlinked_node* next;
    for (struct dlinked_node* p=lst->head; p!=NULL; p=next) {
        next = p->next;
        DELETE(p);
    }
}
