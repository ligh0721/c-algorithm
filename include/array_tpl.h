//
// Created by t5w0rd on 19-4-22.
//

#ifndef ALGORITHM_ARRAY_TPL_H
#define ALGORITHM_ARRAY_TPL_H

#include <string.h>
#include <limits.h>
#include <assert.h>


#define TPL_ARRAY_MAX_SIZE (LONG_MAX-16)

#define ARRAY_DECLARE(ValueType) \
typedef struct ValueType##_array ValueType##_ARRAY;\
\
ValueType##_ARRAY* open_##ValueType##_array(long cap);\
ValueType##_ARRAY* open_##ValueType##_array_by_data(ValueType arr[], long cap);\
ValueType##_ARRAY* reopen_##ValueType##_array(ValueType##_ARRAY* arr, long cap);\
ValueType##_ARRAY* assign_##ValueType##_array(ValueType##_ARRAY* arr);\
void close_##ValueType##_array(ValueType##_ARRAY* arr);\
long ValueType##_array_len(ValueType##_ARRAY* arr);\
long ValueType##_array_cap(ValueType##_ARRAY* arr);\
ValueType* ValueType##_array_data(ValueType##_ARRAY* arr);\
ValueType ValueType##_array_get(ValueType##_ARRAY *arr, long index);\
ValueType ValueType##_array_set(ValueType##_ARRAY *arr, long index, ValueType value);\
long ValueType##_array_ref(ValueType##_ARRAY *arr);\
\
typedef struct ValueType##_slice ValueType##_SLICE;\
\
ValueType##_SLICE* open_##ValueType##_slice(long len, long cap);\
ValueType##_SLICE* open_##ValueType##_slice_by_array(ValueType##_ARRAY* arr, long start, long end);\
ValueType##_SLICE* open_##ValueType##_slice_by_slice(ValueType##_SLICE* sli, long start, long end);\
void close_##ValueType##_slice(ValueType##_SLICE* sli);\
long ValueType##_slice_len(ValueType##_SLICE* sli);\
long ValueType##_slice_cap(ValueType##_SLICE* sli);\
ValueType* ValueType##_slice_data(ValueType##_SLICE *sli);\
long ValueType##_slice_pos(ValueType##_SLICE* sli);\
ValueType ValueType##_slice_get(ValueType##_SLICE *sli, long index);\
ValueType ValueType##_slice_set(ValueType##_SLICE *sli, long index, ValueType value);\
void ValueType##_slice_append(ValueType##_SLICE *sli, ValueType value);\
void ValueType##_slice_insert(ValueType##_SLICE *sli, long index, ValueType value);\
ValueType ValueType##_slice_pop(ValueType##_SLICE *sli, long index);\
ValueType##_ARRAY* ValueType##_slice_array_ref(ValueType##_SLICE *sli);

#define ARRAY_DEFINE(ValueType) \
struct ValueType##_array {\
    long cap;\
    long ref;\
    ValueType data[];\
};\
\
ValueType##_ARRAY* open_##ValueType##_array(long cap) {\
    assert(cap >= 0);\
    struct ValueType##_array* ret = NEW2(struct ValueType##_array, sizeof(ValueType)*cap);\
    assert(ret != NULL);\
    ret->cap = cap;\
    ret->ref = 1;\
    return ret;\
}\
\
ValueType##_ARRAY* open_##ValueType##_array_by_data(ValueType data[], long cap) {\
    struct ValueType##_array* ret = open_##ValueType##_array(cap);\
    memcpy(ret->data, data, sizeof(ValueType)*cap);\
    return ret;\
}\
\
ValueType##_ARRAY* reopen_##ValueType##_array(ValueType##_ARRAY* arr, long cap) {\
    assert(arr != NULL);\
    assert(cap > 0);\
    struct ValueType##_array* ret = RENEW2(arr, struct ValueType##_array, sizeof(ValueType)*cap);\
    assert(ret != NULL);\
    ret->cap = cap;\
    return ret;\
}\
\
ValueType##_ARRAY* assign_##ValueType##_array(ValueType##_ARRAY* arr) {\
    assert(arr != NULL);\
    assert(arr->ref > 0);\
    arr->ref++;\
    return arr;\
}\
\
void close_##ValueType##_array(ValueType##_ARRAY* arr) {\
    assert(arr != NULL);\
    assert(arr->ref > 0);\
    if (--(arr->ref) == 0) {\
        DELETE(arr);\
    }\
}\
\
long ValueType##_array_len(ValueType##_ARRAY* arr) {\
    assert(arr != NULL);\
    return arr->cap;\
}\
\
long ValueType##_array_cap(ValueType##_ARRAY* arr) {\
    assert(arr != NULL);\
    return arr->cap;\
}\
\
ValueType* ValueType##_array_data(ValueType##_ARRAY* arr) {\
    assert(arr != NULL);\
    return arr->data;\
}\
\
ValueType ValueType##_array_get(ValueType##_ARRAY *arr, long index) {\
    assert(arr != NULL);\
    assert(index < arr->cap);\
    return arr->data[index];\
}\
\
ValueType ValueType##_array_set(ValueType##_ARRAY *arr, long index, ValueType value) {\
    assert(arr != NULL);\
    assert(index < arr->cap);\
    ValueType ret = arr->data[index];\
    arr->data[index] = value;\
    return ret;\
}\
\
long ValueType##_array_ref(ValueType##_ARRAY* arr) {\
    assert(arr != NULL);\
    return arr->ref;\
}\
\
\
struct ValueType##_slice {\
    struct ValueType##_array* data;\
    long pos;\
    long len;\
};\
\
ValueType##_SLICE* open_##ValueType##_slice(long len, long cap) {\
    assert(len <= cap);\
    struct ValueType##_slice* ret = NEW(struct ValueType##_slice);\
    ret->data = open_##ValueType##_array(cap);\
    ret->pos = 0;\
    ret->len = len;\
    return ret;\
}\
\
ValueType##_SLICE* open_##ValueType##_slice_by_array(ValueType##_ARRAY* arr, long start, long end) {\
    assert(arr != NULL);\
    assert(start <= end);\
    assert(end <= arr->cap);\
    struct ValueType##_slice* ret = NEW(struct ValueType##_slice);\
    ret->data = assign_##ValueType##_array(arr);\
    ret->pos = start;\
    ret->len = end - start;\
    return ret;\
}\
\
ValueType##_SLICE* open_##ValueType##_slice_by_slice(ValueType##_SLICE* sli, long start, long end) {\
    assert(sli != NULL);\
    assert(start <= end);\
    assert(sli->pos+end <= sli->data->cap);\
    struct ValueType##_slice* ret = NEW(struct ValueType##_slice);\
    ret->data = assign_##ValueType##_array(sli->data);\
    ret->pos = sli->pos + start;\
    ret->len = end - start;\
    return ret;\
}\
\
void close_##ValueType##_slice(ValueType##_SLICE* sli) {\
    assert(sli != NULL);\
    close_##ValueType##_array(sli->data);\
    DELETE(sli);\
}\
\
long ValueType##_slice_len(ValueType##_SLICE* sli) {\
    assert(sli != NULL);\
    return sli->len;\
}\
\
long ValueType##_slice_cap(ValueType##_SLICE* sli) {\
    assert(sli != NULL);\
    return sli->data->cap - sli->pos;\
}\
\
ValueType* ValueType##_slice_data(ValueType##_SLICE *sli) {\
    assert(sli != NULL);\
    return sli->data->data + sli->pos;\
}\
\
long ValueType##_slice_pos(ValueType##_SLICE* sli) {\
    assert(sli != NULL);\
    return sli->pos;\
}\
\
ValueType ValueType##_slice_get(ValueType##_SLICE *sli, long index) {\
    assert(sli != NULL);\
    assert(index < sli->len);\
    return sli->data->data[sli->pos+index];\
}\
\
ValueType ValueType##_slice_set(ValueType##_SLICE *sli, long index, ValueType value) {\
    assert(sli != NULL);\
    assert(index < sli->len);\
    ValueType* retp = sli->data->data + sli->pos + index;\
    ValueType ret = *retp;\
    *retp = value;\
    return ret;\
}\
\
void ValueType##_slice_grow(ValueType##_SLICE* sli, long mincap) {\
    assert(sli != NULL);\
    mincap -= sli->pos;\
    long old_cap = sli->data->cap - sli->pos;\
    long new_cap = old_cap + (old_cap >> 1);\
    if (new_cap < mincap) {\
        new_cap = mincap;\
    }\
    if (new_cap > TPL_ARRAY_MAX_SIZE) {\
        new_cap = mincap > TPL_ARRAY_MAX_SIZE ? LONG_MAX : TPL_ARRAY_MAX_SIZE;\
    }\
    ValueType##_ARRAY* new_arr = open_##ValueType##_array(new_cap);\
    memcpy(new_arr->data, sli->data->data+sli->pos, sizeof(ValueType)*old_cap);\
    close_##ValueType##_array(sli->data);\
    sli->data = new_arr;\
    sli->pos = 0;\
}\
\
void ValueType##_slice_append(ValueType##_SLICE* sli, ValueType value) {\
    assert(sli != NULL);\
    long pos = sli->pos + sli->len;\
    if (pos == sli->data->cap) {\
        ValueType##_slice_grow(sli, sli->data->cap+1);\
        pos = sli->len;\
    }\
    sli->data->data[pos] = value;\
    sli->len++;\
}\
\
void ValueType##_slice_insert(ValueType##_SLICE *sli, long index, ValueType value) {\
    assert(sli != NULL);\
    assert(index <= sli->len);\
    if (sli->pos+sli->len == sli->data->cap) {\
        ValueType##_slice_grow(sli, sli->data->cap+1);\
    }\
    ValueType* arr = sli->data->data + sli->pos;\
    long tomove = sli->len - index;\
    if (tomove > 0) {\
        memmove(arr+1, arr, sizeof(ValueType)*tomove);\
    }\
    *(arr + index) = value;\
    sli->len++;\
}\
\
ValueType ValueType##_slice_pop(ValueType##_SLICE *sli, long index) {\
    assert(sli != NULL);\
    assert(index < sli->len);\
    ValueType* retp = sli->data->data + sli->pos + index;\
    ValueType ret = *retp;\
    long next = index + 1;\
    if (next < sli->len) {\
        memcpy(retp, retp+1, sizeof(ValueType)*(sli->len-next));\
    }\
    sli->len--;\
    return ret;\
}\
\
ValueType##_ARRAY* ValueType##_slice_array_ref(ValueType##_SLICE *sli) {\
    assert(sli != NULL);\
    return sli->data;\
}

#endif //ALGORITHM_ARRAY_TPL_H
