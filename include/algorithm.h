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

extern const VALUE NULL_VALUE;

#define NEW(TYPE) ((TYPE*)malloc(sizeof(TYPE)))
#define NEW0(size) (malloc(size))
#define NEW2(TYPE, append) ((TYPE*)malloc(sizeof(TYPE)+(append)))
#define NEW3(TYPE, count) ((TYPE*)malloc(sizeof(TYPE)*(count)))
#define RENEW2(p, TYPE, append) ((TYPE*)realloc((p), sizeof(TYPE)+(append)))
#define RENEW3(p, TYPE, count) ((TYPE*)realloc((p), sizeof(TYPE)*(count)))
#define DELETE(p) free((p))

inline void swap(VALUE* a, VALUE* b) {
    VALUE t = *a;
    *a = *b;
    *b = t;
}

typedef int (*COMPARE)(VALUE, VALUE);

int asc_order_int(VALUE a, VALUE b);
int desc_order_int(VALUE a, VALUE b);

typedef void (*PRINT_VALUE)(VALUE value);

typedef int (*TRAVERSE)(VALUE value, void* param);

typedef void* (*ALLOC)(size_t size);
typedef void (*FREE)(void* p);
typedef struct ALLOCATOR {
    ALLOC alloc;
    FREE free;
} ALLOCATOR;

extern const ALLOCATOR NULL_ALLOCATOR;

#endif //ALGORITHM_ALGORITHM_H
