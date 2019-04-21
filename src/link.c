//
// Created by t5w0rd on 19-4-14.
//

#include <stdlib.h>
#include <assert.h>
#include "link.h"


// linked list
struct llist {
    struct lnode head;
    struct lnode* tail;
    long length;
    ALLOCATOR allocator;
};

static inline void* new(LLIST* lst, size_t size) {
    return lst->allocator.alloc != NULL ? lst->allocator.alloc(size) : NEW0(size);
}

static inline void delete(LLIST* lst, void* p) {
    if (lst->allocator.free != NULL) {
        lst->allocator.free(p);
    } else if (lst->allocator.alloc == NULL) {
        DELETE(p);
    }
}

LLIST* open_llist() {
    struct llist* ret = NEW(struct llist);
    assert(ret != NULL);
    ret->head.value = NULL_VALUE;
    ret->head.next = NULL;
    ret->tail = &ret->head;
    ret->length = 0;
    ret->allocator = NULL_ALLOCATOR;
    return ret;
}

LLIST* open_llist_with_allocator(ALLOCATOR allocator) {
    assert(allocator.alloc != NULL);
    struct llist* ret = (struct llist*)allocator.alloc(sizeof(struct llist));
    assert(ret != NULL);
    ret->head.value = NULL_VALUE;
    ret->head.next = NULL;
    ret->tail = &ret->head;
    ret->length = 0;
    ret->allocator = allocator;
    return ret;
}

void close_llist(LLIST *lst) {
    assert(lst != NULL);
    llist_clear(lst);
    delete(lst, lst);
}

void llist_clear(LLIST* lst) {
    assert(lst != NULL);
    for (struct lnode* node=lst->head.next; node!=NULL; ) {
        struct lnode* del = node;
        node = node->next;
        delete(lst, del);
    }
    lst->head.next = NULL;
    lst->tail = &lst->head;
    lst->length = 0;
}

long llist_len(LLIST* lst) {
    assert(lst != NULL);
    return lst->length;
}

struct lnode* llist_front_node(LLIST* lst) {
    assert(lst != NULL);
    return lst->head.next;
}

void llist_traversal(LLIST* lst, TRAVERSE traverse, void* param) {
    assert(lst != NULL);
    for (struct lnode* node=lst->head.next; node!=NULL; node=node->next) {
        if (traverse(node->value, param)) {
            break;
        }
    }
}

void llist_push_back(LLIST* lst, VALUE value) {
    assert(lst != NULL);
    struct lnode* node = (struct lnode*)new(lst, sizeof(struct lnode));
    node->value = value;
    node->next = NULL;
    lst->tail = lst->tail->next = node;
    lst->length++;
}

// double linked list
struct dllist {
    struct dlnode head;
    struct dlnode* tail;
    long length;
};

DLLIST* open_dllist() {
    struct dllist* ret = NEW(struct dllist);
    assert(ret != NULL);
    ret->head.value = NULL_VALUE;
    ret->head.prev = &ret->head;
    ret->head.next = NULL;
    ret->tail = &ret->head;
    ret->length = 0;
    return ret;
}

void close_dllist(DLLIST *lst) {
    assert(lst != NULL);
    dllist_clear(lst);
    DELETE(lst);
}

void dllist_clear(DLLIST* lst) {
    assert(lst != NULL);
    for (struct dlnode* node=lst->head.next; node!=NULL; ) {
        struct dlnode* del = node;
        node = node->next;
        DELETE(del);
    }
    lst->head.next = NULL;
    lst->tail = &lst->head;
    lst->length = 0;
}

long dllist_len(DLLIST* lst) {
    assert(lst != NULL);
    return lst->length;
}

void dlist_traversal(DLLIST* lst, int reverse, TRAVERSE traverse, void* param) {
    assert(lst != NULL);
    if (reverse) {
        for (struct dlnode* node=lst->tail; node!=&lst->head; node=node->prev) {
            if (traverse(node->value, param)) {
                break;
            }
        }
    } else {
        for (struct dlnode* node=lst->head.next; node!=NULL; node=node->next) {
            if (traverse(node->value, param)) {
                break;
            }
        }
    }
}

void dllist_push_back(DLLIST* lst, VALUE value) {
    assert(lst != NULL);
    struct dlnode* node = NEW(struct dlnode);
    node->value = value;
    node->next = NULL;
    node->prev = lst->tail;
    lst->tail = lst->tail->next = node;
    lst->length++;
}
