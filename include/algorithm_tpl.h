//
// Created by t5w0rd on 19-4-22.
//

#ifndef ALGORITHM_ALGORITHM_TPL_H
#define ALGORITHM_ALGORITHM_TPL_H

#define NULL_VALUE_DEF(ValueType, value) \
const ValueType##_VALUE ValueType##_NULL_VALUE = value;

#define COMPARE_DEF(ValueType) \
typedef int (*ValueType##_COMPARE)(ValueType##_VALUE, ValueType##_VALUE);

#define TRAVERSE_DEF(ValueType) \
typedef int (*ValueType##_TRAVERSE)(ValueType##_VALUE value, void* param);

#endif //ALGORITHM_ALGORITHM_TPL_H
