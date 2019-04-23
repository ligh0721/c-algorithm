//
// Created by t5w0rd on 19-4-14.
//

#ifndef ALGORITHM_LINK_H
#define ALGORITHM_LINK_H

#include "algorithm.h"


struct lnode {
    VALUE value;
    struct lnode* next;
};

typedef struct llist LLIST;

LLIST* open_llist();
LLIST* open_llist_with_allocator(ALLOCATOR allocator);
void close_llist(LLIST *lst);
void llist_clear(LLIST* lst);
long llist_len(LLIST* lst);
struct lnode* llist_front_node(LLIST* lst);
struct lnode* llist_back_node(LLIST* lst);
struct lnode* llist_before_front_node(LLIST *lst);
int llist_is_node_before_front(LLIST *lst, struct lnode *back);
void llist_traversal(LLIST* lst, TRAVERSE traverse, void* param);
void llist_push_back(LLIST* lst, VALUE value);

struct dlnode {
    VALUE value;
    struct dlnode* next;
    struct dlnode* prev;
};

typedef struct dllist DLLIST;

DLLIST* open_dllist();
void close_dllist(DLLIST *lst);
void dllist_clear(DLLIST* lst);
long dllist_len(DLLIST* lst);
void dlist_traversal(DLLIST* lst, int reverse, TRAVERSE traverse, void* param);
void dllist_push_back(DLLIST* lst, VALUE value);

#endif //ALGORITHM_LINK_H
