//
// Created by t5w0rd on 19-4-14.
//

#ifndef ALGORITHM_HEAP_H
#define ALGORITHM_HEAP_H

#include "algorithm.h"


typedef struct heap HEAP;

HEAP* open_heap(COMPARE compare, long cap);
HEAP* open_heap_by_data(COMPARE compare, const VALUE data[], long cap);
void close_heap(HEAP* hp);
void heap_push(HEAP* hp, const VALUE value);
VALUE heap_pop(HEAP* hp);
const VALUE heap_peek(HEAP* hp);

SLICE* _heap_data(HEAP* hp);

#endif //ALGORITHM_HEAP_H
