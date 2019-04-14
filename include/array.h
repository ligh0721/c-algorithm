//
// Created by t5w0rd on 19-4-14.
//

#ifndef ALGORITHM_ARRAY_H
#define ALGORITHM_ARRAY_H

#include "algorithm.h"

typedef struct array ARRAY;

ARRAY* open_array(size_t cap);
ARRAY* open_array_by_data(const VALUE arr[], size_t cap);
ARRAY* reopen_array(ARRAY* arr, size_t cap);
ARRAY* assign_array(ARRAY* arr);
void close_array(ARRAY* arr);
size_t array_len(ARRAY* arr);
size_t array_cap(ARRAY* arr);
VALUE array_get(ARRAY *arr, size_t index);
void array_set(ARRAY *arr, size_t index, const VALUE value);


typedef struct slice SLICE;

SLICE* open_slice(size_t len, size_t cap);
SLICE* open_slice_by_array(ARRAY* arr, size_t start, size_t end);
SLICE* open_slice_by_slice(SLICE* sli, size_t start, size_t end);
void close_slice(SLICE* sli);
size_t slice_len(SLICE* sli);
size_t slice_cap(SLICE* sli);
VALUE slice_get(SLICE *sli, size_t index);
void slice_set(SLICE *sli, size_t index, const VALUE value);
void slice_append(SLICE* sli, const VALUE value);

#endif //ALGORITHM_ARRAY_H
