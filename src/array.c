//
// Created by t5w0rd on 19-4-14.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "array.h"


struct array {
    long cap;
    long ref;
    VALUE data[0];
};

ARRAY* open_array(long cap) {
    assert(cap > 0);
    struct array* ret = NEW2(struct array, sizeof(VALUE)*cap);
    assert(ret != NULL);
    ret->cap = cap;
    ret->ref = 1;
    return ret;
}

ARRAY* open_array_by_data(const VALUE data[], long cap) {
    struct array* ret = open_array(cap);
    memcpy(ret->data, data, sizeof(VALUE)*cap);
    return ret;
}

ARRAY* reopen_array(ARRAY* arr, long cap) {
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

long array_len(ARRAY* arr) {
    assert(arr != NULL);
    return arr->cap;
}

long array_cap(ARRAY* arr) {
    assert(arr != NULL);
    return arr->cap;
}

VALUE* array_data(ARRAY* arr) {
    assert(arr != NULL);
    return arr->data;
}

VALUE array_get(ARRAY *arr, long index) {
    assert(arr != NULL);
    assert(index < arr->cap);
    return arr->data[index];
}

VALUE array_set(ARRAY *arr, long index, const VALUE value) {
    assert(arr != NULL);
    assert(index < arr->cap);
    VALUE ret = arr->data[index];
    arr->data[index] = value;
    return ret;
}


struct slice {
    struct array* arr;
    long pos;
    long len;
};

SLICE* open_slice(long len, long cap) {
    assert(len <= cap);
    struct slice* ret = NEW(struct slice);
    if (cap < 1) {
        cap = 1;
    }
    ret->arr = open_array(cap);
    ret->pos = 0;
    ret->len = len;
    return ret;
}

SLICE* open_slice_by_array(ARRAY* arr, long start, long end) {
    assert(arr != NULL);
    assert(start <= end);
    assert(end <= arr->cap);
    struct slice* ret = NEW(struct slice);
    ret->arr = assign_array(arr);
    ret->pos = start;
    ret->len = end - start;
    return ret;
}

SLICE* open_slice_by_slice(SLICE* sli, long start, long end) {
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

long slice_len(SLICE* sli) {
    assert(sli != NULL);
    return sli->len;
}

long slice_cap(SLICE* sli) {
    assert(sli != NULL);
    return sli->arr->cap - sli->pos;
}

VALUE* slice_data(SLICE *sli) {
    assert(sli != NULL);
    return sli->arr->data + sli->pos;
}

VALUE slice_get(SLICE *sli, long index) {
    assert(sli != NULL);
    assert(index < sli->len);
    return sli->arr->data[sli->pos+index];
}

VALUE slice_set(SLICE *sli, long index, const VALUE value) {
    assert(sli != NULL);
    assert(index < sli->len);
    VALUE* retp = sli->arr->data + sli->pos + index;
    VALUE ret = *retp;
    *retp = value;
    return ret;
}

void slice_append(SLICE *sli, const VALUE value) {
    assert(sli != NULL);
    ARRAY* arr = sli->arr;
    long pos = sli->pos + sli->len;
    if (pos == arr->cap) {
        ARRAY* new_arr = open_array(arr->cap*2);
        memcpy(new_arr->data, arr->data, sizeof(VALUE)*arr->cap);
        close_array(arr);
        arr = sli->arr = new_arr;
//        arr = reopen_array(arr, arr->cap*2);
        printf("realloc array data\n");
    }
    arr->data[pos] = value;
    sli->len++;
}

VALUE slice_remove(SLICE* sli, long index) {
    assert(sli != NULL);
    assert(index < sli->len);
    VALUE* retp = sli->arr->data + sli->pos + index;
    VALUE ret = *retp;
    long next = index + 1;
    if (next < sli->len) {
        memcpy(retp, retp+1, sizeof(VALUE)*(sli->len-next));
    }
    sli->len--;
    return ret;
}
