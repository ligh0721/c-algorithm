//
// Created by t5w0rd on 19-4-13.
//

#ifndef ALGORITHM_ALGORITHM_H
#define ALGORITHM_ALGORITHM_H

#include <stdlib.h>


typedef union VALUE {
    long int_value;
    double float_value;
    void* ptr_value;
} VALUE;

VALUE int_value(long value);
VALUE float_value(double value);
VALUE ptr_value(void* value);

#define is_null_value(obj) ((obj).ptr_value==NULL)
#define NULL_VALUE ((VALUE)NULL)

#define NEW(TYPE) ((TYPE*)malloc(sizeof(TYPE)))
#define NEW2(TYPE, append) ((TYPE*)malloc(sizeof(TYPE)+(append)))
#define RENEW2(p, TYPE, append) ((TYPE*)realloc((p), sizeof(TYPE)+(append)))
#define DELETE(p) free((p))

typedef int (*COMPARE)(const VALUE, const VALUE);

inline void swap(VALUE* a, VALUE* b) {
    VALUE t = *a;
    *a = *b;
    *b = t;
}

int asc_order_int(const VALUE a, const VALUE b);
int desc_order_int(const VALUE a, const VALUE b);
void print_array_int(const VALUE arr[], long size);

#endif //ALGORITHM_ALGORITHM_H
