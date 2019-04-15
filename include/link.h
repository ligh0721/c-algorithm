//
// Created by t5w0rd on 19-4-14.
//

#ifndef ALGORITHM_LINK_H
#define ALGORITHM_LINK_H

#include "algorithm.h"


struct linked_node {
    VALUE value;
    struct linked_node* next;
};

struct linked_node* open_linked_node(VALUE value, struct linked_node* next);


struct dlinked_node {
    VALUE value;
    struct dlinked_node* next;
    struct dlinked_node* prev;
};

struct dlinked_node* open_dlinked_node(VALUE value, struct dlinked_node* prev, struct dlinked_node* next);
void close_dlinked_node(struct dlinked_node* node);


typedef struct dlinked_list DLINKED_LIST;

DLINKED_LIST* open_dlinked_list();
void close_dlinked_list(DLINKED_LIST* lst);

#endif //ALGORITHM_LINK_H
