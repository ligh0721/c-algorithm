//
// Created by t5w0rd on 19-4-13.
//

#include "algorithm.h"

int int_asc_order(const Object a, const Object b) {
    return a.int_value <= b.int_value;
}

int int_desc_order(const Object a, const Object b) {
    return a.int_value >= b.int_value;
}