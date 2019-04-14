//
// Created by t5w0rd on 19-4-14.
//

#include <stdlib.h>
#include "algorithm.h"
#include "link.h"




void close_all_link_node(struct link_node *from) {
    struct link_node* next;
    for (struct link_node* p=from; p!=NULL; p=next) {
        next = p->next;
        free(p);
    }
}
