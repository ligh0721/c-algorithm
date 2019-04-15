//
// Created by t5w0rd on 2019-04-15.
//

#ifndef ALGORITHM_DEQUE_H
#define ALGORITHM_DEQUE_H


typedef struct deque DEQUE;

DEQUE* open_deque(long cap);
void close_deque(DEQUE* dq);
void deque_grow(DEQUE* dq, long mincap);
long deque_len(DEQUE* dq);
long deque_cap(DEQUE* dq);
VALUE deque_get(DEQUE* dq, long index);
VALUE deque_set(DEQUE* dq, long index, const VALUE value);
void deque_push_front(DEQUE* dq, const VALUE value);
VALUE deque_pop_front(DEQUE* dq, int* empty);
VALUE deque_front(DEQUE* dq);
void deque_push_back(DEQUE* dq, const VALUE value);
VALUE deque_pop_back(DEQUE* dq, int* empty);
VALUE deque_back(DEQUE* dq);


#endif //ALGORITHM_DEQUE_H
