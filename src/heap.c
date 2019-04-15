//
// Created by t5w0rd on 19-4-14.
//

#include <assert.h>
#include "array.h"
#include "heap.h"


struct heap {
    COMPARE compare;
    SLICE* data;
};

void _heap_sift_up(VALUE arr[], long size, long index, COMPARE compare) {
    VALUE k = arr[index];
    long i, p;
    for (i=index; i>0; i=p) {
        p = (i - 1) >> 1;
        if (compare(arr[p], k)) {
            break;
        }
        arr[i] = arr[p];
    }
    if (i != index) {
        arr[i] = k;
    }
}

void _heap_sift_down(VALUE arr[], long size, long index, COMPARE compare) {
    VALUE k = arr[index];
    long i = index;
    for (long c=(i<<1)+1; c<size; c=(i<<1)+1) {
        long r = c + 1;
        if (r < size && !compare(arr[c], arr[r])) {
            c = r;
        }
        if (compare(k, arr[c])) {
            break;
        }
        arr[i] = arr[c];
        i = c;
    }
    if (i != index) {
        arr[i] = k;
    }
}

void _heap_build(HEAP* hp) {
    long len = slice_len(hp->data);
    VALUE* arr = slice_data(hp->data);
    for (long i=(len>>1)-1; i>=0; --i) {
        _heap_sift_down(arr, len, i, hp->compare);
    }
}

HEAP* open_heap(COMPARE compare, long cap) {
    struct heap* ret = NEW(struct heap);
    assert(ret != NULL);
    ret->compare = compare;
    ret->data = open_slice(0, cap);
    return ret;
}

HEAP* open_heap_by_data(COMPARE compare, const VALUE data[], long cap) {
    struct heap* ret = NEW(struct heap);
    assert(ret != NULL);
    ret->compare = compare;
    ARRAY* arr = open_array_by_data(data, cap);
    ret->data = open_slice_by_array(arr, 0, cap);
    close_array(arr);
    _heap_build(ret);
    return ret;
}

void close_heap(HEAP* hp) {
    assert(hp != NULL);
    close_slice(hp->data);
    DELETE(hp);
}

long heap_len(HEAP* hp) {
    assert(hp != NULL);
    return slice_len(hp->data);
}

void heap_push(HEAP* hp, const VALUE value) {
    assert(hp != NULL);
    long index = slice_len(hp->data);
    slice_append(hp->data, value);
    _heap_sift_up(slice_data(hp->data), slice_len(hp->data), index, hp->compare);
}

VALUE heap_pop(HEAP* hp, int* empty) {
    assert(hp != NULL);
    long len = slice_len(hp->data);
    if (len == 0) {
        if (empty != NULL) {
            *empty = 1;
        }
        return NULL_VALUE;
    }
    if (len == 1) {
        return slice_pop(hp->data, len - 1);
    }
    VALUE ret = slice_set(hp->data, 0, slice_pop(hp->data, len - 1));
    _heap_sift_down(slice_data(hp->data), len-1, 0, hp->compare);
    return ret;
}

const VALUE heap_top(HEAP *hp) {
    assert(hp != NULL);
    if (slice_len(hp->data) == 0) {
        return NULL_VALUE;
    }
    return slice_get(hp->data, 0);
}

SLICE* _heap_data(HEAP* hp) {
    assert(hp != NULL);
    return hp->data;
}
