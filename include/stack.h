//
// Created by t5w0rd on 19-4-14.
//

#ifndef ALGORITHM_STACK_H
#define ALGORITHM_STACK_H

#include "algorithm.h"
#include "array.h"


typedef struct stack STACK;

STACK* open_stack(long cap);
void close_stack(STACK* st);
long stack_len(STACK* st);
void stack_push(STACK* st, VALUE value);
VALUE stack_pop(STACK* st, int* empty);
VALUE stack_top(STACK* st);
SLICE* stack_data(STACK* st);

#endif //ALGORITHM_STACK_H
