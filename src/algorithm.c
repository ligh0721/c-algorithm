//
// Created by t5w0rd on 19-4-13.
//

#include <stdio.h>
#include "algorithm.h"


VALUE int_value(long value) {
    VALUE ret;
    ret.int_value = value;
    return ret;
}

VALUE float_value(double value) {
    VALUE ret;
    ret.float_value = value;
    return ret;
}

VALUE ptr_value(void* value) {
    VALUE ret;
    ret.ptr_value = value;
    return ret;
}

const VALUE NULL_VALUE = {};

int asc_order_int(VALUE a, VALUE b) {
    return a.int_value - b.int_value;
}

int desc_order_int(VALUE a, VALUE b) {
    return b.int_value - a.int_value;
}

const ALLOCATOR NULL_ALLOCATOR = {};
