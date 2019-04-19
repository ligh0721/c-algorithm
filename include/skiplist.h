//
// Created by t5w0rd on 19-4-18.
//

#ifndef ALGORITHM_SKIPLIST_H
#define ALGORITHM_SKIPLIST_H

#include "algorithm.h"
#include "array.h"


typedef struct skiplist SKIPLIST;

SKIPLIST* open_skiplist(COMPARE compare1, COMPARE compare2);
void close_skiplist(SKIPLIST* sl);
void skiplist_clear(SKIPLIST* sl);
void skiplist_set(SKIPLIST* sl, VALUE value);
VALUE skiplist_get(SKIPLIST* sl, VALUE key, int* ok);
void skiplist_range(SKIPLIST* sl, SLICE* data, VALUE key1, VALUE key2, long limit);

#endif //ALGORITHM_SKIPLIST_H
