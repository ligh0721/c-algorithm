//
// Created by t5w0rd on 19-4-13.
//

#ifndef ALGORITHM_ALGORITHM_H
#define ALGORITHM_ALGORITHM_H


typedef union OBJECT {
    long int_value;
    double float_value;
    void* ptr_value;
} VALUE;

#define is_null_object(obj) ((obj).ptr_value==NULL)
#define NULL_OBJECT ((VALUE)NULL)

#define NEW(TYPE) ((TYPE*)malloc(sizeof(TYPE)))
#define NEW2(TYPE, append) ((TYPE*)malloc(sizeof(TYPE)+(append)))
#define RENEW2(p, TYPE, append) ((TYPE*)realloc((p), sizeof(TYPE)+(append)))
#define DELETE(p) free((p))

typedef int compare_function(const VALUE, const VALUE);

inline void swap(VALUE* a, VALUE* b) {
    VALUE t = *a;
    *a = *b;
    *b = t;
}

int asc_order_int(const VALUE a, const VALUE b);
int desc_order_int(const VALUE a, const VALUE b);
void print_array_int(const VALUE arr[], size_t size);

#endif //ALGORITHM_ALGORITHM_H
