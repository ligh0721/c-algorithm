//
// Created by t5w0rd on 19-4-14.
//

#ifndef ALGORITHM_LINK_H
#define ALGORITHM_LINK_H

#include "algorithm.h"


struct link_node {
    VALUE value;
    struct link_node* next;
};

inline struct link_node* open_link_node(VALUE value, struct link_node *next) {
    struct link_node* ret = NEW(struct link_node);
    ret->value = value;
    ret->next = next;
    return ret;
}

void close_all_link_node(struct link_node *from);

#endif //ALGORITHM_LINK_H
