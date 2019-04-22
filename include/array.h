//
// Created by t5w0rd on 19-4-14.
//

#ifndef ALGORITHM_ARRAY_H
#define ALGORITHM_ARRAY_H

#include <limits.h>
#include "algorithm.h"

#define ARRAY_MAX_SIZE (LONG_MAX-16)


typedef struct array ARRAY;

ARRAY* open_array(long cap);
ARRAY* open_array_by_data(VALUE arr[], long cap);
ARRAY* reopen_array(ARRAY* arr, long cap);
ARRAY* assign_array(ARRAY* arr);
void close_array(ARRAY* arr);
long array_len(ARRAY* arr);
long array_cap(ARRAY* arr);
VALUE* array_data(ARRAY* arr);
VALUE array_get(ARRAY *arr, long index);
VALUE array_set(ARRAY *arr, long index, VALUE value);

typedef struct slice SLICE;

SLICE* open_slice(long len, long cap);
SLICE* open_slice_by_array(ARRAY* arr, long start, long end);
SLICE* open_slice_by_slice(SLICE* sli, long start, long end);
void close_slice(SLICE* sli);
long slice_len(SLICE* sli);
long slice_cap(SLICE* sli);
VALUE* slice_data(SLICE *sli);
VALUE slice_get(SLICE *sli, long index);
VALUE slice_set(SLICE *sli, long index, VALUE value);
void slice_append(SLICE *sli, VALUE value);
void slice_push(SLICE *sli, long index, VALUE value);
VALUE slice_pop(SLICE *sli, long index);

#endif //ALGORITHM_ARRAY_H
