//
// Created by t5w0rd on 19-4-22.
//

#ifndef ALGORITHM_ALGORITHM_TPL_H
#define ALGORITHM_ALGORITHM_TPL_H

#define EMPTY_VALUE_DEFINE(ValueType, value) \
const ValueType ValueType##_EMPTY = value;

#define COMPARE_DEFINE(ValueType) \
typedef int (*ValueType##_COMPARE)(ValueType, ValueType);

#define TRAVERSE_DEFINE(ValueType) \
typedef int (*ValueType##_TRAVERSE)(const ValueType* value, void* param);

#endif //ALGORITHM_ALGORITHM_TPL_H
