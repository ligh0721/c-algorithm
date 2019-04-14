//
// Created by t5w0rd on 19-4-13.
//

#ifndef ALGORITHM_ALGORITHM_H
#define ALGORITHM_ALGORITHM_H


typedef union {
    long int_value;
    double float_value;
    void* pointer_value;
} Object;

typedef int compare_function(const Object, const Object);

inline void swap(Object* a, Object* b) {
    Object t = *a;
    *a = *b;
    *b = t;
}

int int_asc_order(const Object a, const Object b);
int int_desc_order(const Object a, const Object b);

#endif //ALGORITHM_ALGORITHM_H
