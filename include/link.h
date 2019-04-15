//
// Created by t5w0rd on 19-4-14.
//

#ifndef ALGORITHM_LINK_H
#define ALGORITHM_LINK_H

#include "algorithm.h"


struct linked_node {
    VALUE value;
    struct linked_node* next;
    struct linked_node* prev;
};

struct linked_node* open_linked_node(VALUE value, struct linked_node* prev, struct linked_node* next);
void close_linked_node(struct linked_node* node);

typedef struct list LIST;

#endif //ALGORITHM_LINK_H
