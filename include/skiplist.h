//
// Created by t5w0rd on 19-4-18.
//

#ifndef ALGORITHM_SKIPLIST_H
#define ALGORITHM_SKIPLIST_H

#include "algorithm.h"


typedef struct skiplist SKIPLIST;

SKIPLIST* open_skiplist(COMPARE compare);
void close_skiplist(SKIPLIST* sl);
void skiplist_clear(SKIPLIST* sl);
void skiplist_set(SKIPLIST* sl, VALUE value);
VALUE skiplist_get(SKIPLIST* sl, VALUE key, int* ok);

#endif //ALGORITHM_SKIPLIST_H
