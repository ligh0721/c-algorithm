//
// Created by t5w0rd on 2019-04-15.
//

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "array.h"
#include "deque.h"


struct deque {
    ARRAY* data;
    long front;
    long back;
};

DEQUE* open_deque(long cap) {
    assert(cap >= 0);
    if (cap < 1) {
        cap = 1;
    }
    assert(cap > 0);
    struct deque* ret = NEW(struct deque);
    assert(ret != NULL);
    ret->data = open_array(cap);
    ret->front = -1;
    ret->back = -1;
    return ret;
}

void close_deque(DEQUE* dq) {
    assert(dq != NULL);
    close_array(dq->data);
    DELETE(dq);
}

void deque_grow(DEQUE* dq, long mincap) {
    assert(dq != NULL);
    VALUE* old_data = array_data(dq->data);
    long old_cap = array_cap(dq->data);
    long new_cap = old_cap + (old_cap >> 1);
    if (new_cap < mincap) {
        new_cap = mincap;
    }
    if (new_cap > ARRAY_MAX_SIZE) {
        new_cap = mincap > ARRAY_MAX_SIZE ? LONG_MAX : ARRAY_MAX_SIZE;
    }

    ARRAY* new_arr = open_array(new_cap);
    VALUE* new_data = array_data(new_arr);
    long off = dq->back - dq->front;
    long len;
    if (off >= 0) {
        len = off + 1;
        memcpy(new_data, old_data+dq->front, sizeof(VALUE)*len);
    } else {
        len = off + old_cap + 1;
        long copied = old_cap - dq->front;
        memcpy(new_data, old_data+dq->front, sizeof(VALUE)*copied);
        memcpy(new_data+copied, old_data, sizeof(VALUE)*(dq->back+1));
    }
    close_array(dq->data);

    dq->data = new_arr;
    dq->front = 0;
    dq->back = len - 1;
    printf("deque_grow: cap: %ld -> %ld\n", old_cap, new_cap);
}

long deque_len(DEQUE* dq) {
    assert(dq != NULL);
    long len = dq->back - dq->front;
    len = len < 0 ? (len + array_cap(dq->data) + 1) : (len + 1);
    return len;
}

long deque_cap(DEQUE* dq) {
    assert(dq != NULL);
    return array_cap(dq->data);
}

VALUE deque_get(DEQUE* dq, long index) {
    assert(dq != NULL);
    assert(index < deque_len(dq));
    index = dq->front + index;
    long index2 = index - array_cap(dq->data);
    if (index2 > 0) {
        index = index2;
    }
    return array_get(dq->data, index);
}

VALUE deque_set(DEQUE* dq, long index, VALUE value) {
    assert(dq != NULL);
    assert(index < deque_len(dq));
    index = dq->front + index;
    long index2 = index - array_cap(dq->data);
    if (index2 > 0) {
        index = index2;
    }
    return array_set(dq->data, index, value);
}

void deque_push_front(DEQUE* dq, VALUE value) {
    assert(dq != NULL);
    if (dq->front == -1) {
        dq->back = 0;
    }
    VALUE* arr = array_data(dq->data);
    long cap = array_cap(dq->data);
    dq->front--;
    if (dq->front == -1) {
        dq->front += cap;
    }
    arr[dq->front] = value;
    long len = dq->back - dq->front;
    len = len < 0 ? (len + cap + 1) : (len + 1);
    if (len == cap) {
        deque_grow(dq, cap+1);
    }
}

VALUE deque_pop_front(DEQUE* dq, int* empty) {
    assert(dq != NULL);
    if (dq->front == -1) {
        if (empty != NULL) {
            *empty = 1;
        }
        return EMPTY_VALUE;
    }
    VALUE ret = array_get(dq->data, dq->front);
    long cap = array_cap(dq->data);
    if (dq->front == dq->back) {
        dq->front = dq->back = -1;
    } else {
        dq->front++;
        if (dq->front == cap) {
            dq->front = 0;
        }
    }
    return ret;
}

VALUE deque_front(DEQUE* dq) {
    assert(dq != NULL);
    if (dq->front == -1) {
        return EMPTY_VALUE;
    }
    return array_get(dq->data, dq->front);
}

void deque_push_back(DEQUE* dq, VALUE value) {
    assert(dq != NULL);
    if (dq->back == -1) {
        dq->front = 0;
    }
    VALUE* arr = array_data(dq->data);
    long cap = array_cap(dq->data);
    dq->back++;
    if (dq->back == cap) {
        dq->back = 0;
    }
    arr[dq->back] = value;
    long len = dq->back - dq->front;
    len = len < 0 ? (len + cap + 1) : (len + 1);
    if (len == cap) {
        deque_grow(dq, cap+1);
    }
}

VALUE deque_pop_back(DEQUE* dq, int* empty) {
    assert(dq != NULL);
    if (dq->back == -1) {
        if (empty != NULL) {
            *empty = 1;
        }
        return EMPTY_VALUE;
    }
    VALUE ret = array_get(dq->data, dq->back);
    if (dq->front == dq->back) {
        dq->front = dq->back = -1;
    } else {
        dq->back--;
        if (dq->back == -1) {
            dq->back += array_cap(dq->data);
        }
    }
    return ret;
}

VALUE deque_back(DEQUE* dq) {
    assert(dq != NULL);
    if (dq->back == -1) {
        return EMPTY_VALUE;
    }
    return array_get(dq->data, dq->back);
}

ARRAY* _deque_data(DEQUE* dq) {
    assert(dq != NULL);
    return dq->data;
}
