//
// Created by t5w0rd on 19-4-14.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "array.h"


struct array {
    size_t cap;
    size_t ref;
    VALUE data[0];
};

ARRAY* open_array(size_t cap) {
    assert(cap > 0);
    struct array* ret = NEW2(struct array, sizeof(VALUE)*cap);
    assert(ret != NULL);
    ret->cap = cap;
    ret->ref = 1;
    return ret;
}

ARRAY* open_array_by_data(const VALUE arr[], size_t cap) {
    struct array* ret = open_array(cap);
    memmove(ret->data, arr, sizeof(VALUE)*cap);
    return ret;
}

ARRAY* reopen_array(ARRAY* arr, size_t cap) {
    assert(arr != NULL);
    assert(cap > 0);
    struct array* ret = RENEW2(arr, struct array, sizeof(VALUE)*cap);
    assert(ret != NULL);
    ret->cap = cap;
    return ret;
}

ARRAY* assign_array(ARRAY* arr) {
    assert(arr != NULL);
    assert(arr->ref > 0);
    arr->ref++;
    return arr;
}

void close_array(ARRAY* arr) {
    assert(arr != NULL);
    assert(arr->ref > 0);
    if (--(arr->ref) == 0) {
        printf("free array data\n");
        DELETE(arr);
    }
}

size_t array_len(ARRAY* arr) {
    assert(arr != NULL);
    return arr->cap;
}

size_t array_cap(ARRAY* arr) {
    assert(arr != NULL);
    return arr->cap;
}

VALUE array_get(ARRAY *arr, size_t index) {
    assert(arr != NULL);
    assert(index < arr->cap);
    return arr->data[index];
}

void array_set(ARRAY *arr, size_t index, const VALUE value) {
    assert(arr != NULL);
    assert(index < arr->cap);
    arr->data[index] = value;
}


struct slice {
    struct array* arr;
    size_t pos;
    size_t len;
};

SLICE* open_slice(size_t len, size_t cap) {
    assert(len <= cap);
    struct slice* ret = NEW(struct slice);
    ret->arr = open_array(cap);
    ret->pos = 0;
    ret->len = len;
    return ret;
}

SLICE* open_slice_by_array(ARRAY* arr, size_t start, size_t end) {
    assert(arr != NULL);
    assert(start <= end);
    assert(end <= arr->cap);
    struct slice* ret = NEW(struct slice);
    ret->arr = assign_array(arr);
    ret->pos = start;
    ret->len = end - start;
    return ret;
}

SLICE* open_slice_by_slice(SLICE* sli, size_t start, size_t end) {
    assert(sli != NULL);
    assert(start <= end);
    assert(sli->pos+end <= sli->arr->cap);
    struct slice* ret = NEW(struct slice);
    ret->arr = assign_array(sli->arr);
    ret->pos = sli->pos + start;
    ret->len = end - start;
    return ret;
}

void close_slice(SLICE* sli) {
    assert(sli != NULL);
    close_array(sli->arr);
    DELETE(sli);
}

size_t slice_len(SLICE* sli) {
    assert(sli != NULL);
    return sli->len;
}

size_t slice_cap(SLICE* sli) {
    assert(sli != NULL);
    return sli->arr->cap - sli->pos;
}

VALUE slice_get(SLICE *sli, size_t index) {
    assert(sli != NULL);
    assert(index < sli->len);
    return sli->arr->data[sli->pos+index];
}

void slice_set(SLICE *sli, size_t index, const VALUE value) {
    assert(sli != NULL);
    assert(index < sli->len);
    sli->arr->data[sli->pos+index] = value;
}

void slice_append(SLICE* sli, const VALUE value) {
    assert(sli != NULL);
    ARRAY* arr = sli->arr;
    size_t pos = sli->pos + sli->len;
    if (pos == arr->cap) {
        ARRAY* new_arr = open_array(arr->cap*2);
        memmove(new_arr->data, arr->data, sizeof(VALUE)*arr->cap);
        close_array(arr);
        arr = sli->arr = new_arr;
//        arr = reopen_array(arr, arr->cap*2);
        printf("realloc array data\n");
    }
    arr->data[pos] = value;
    sli->len++;
}