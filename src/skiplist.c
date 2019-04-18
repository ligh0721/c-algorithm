//
// Created by t5w0rd on 19-4-18.
//

#include <assert.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include "algorithm.h"
#include "skiplist.h"


struct slnode {
    VALUE value;
    struct slnode* prev;
    struct slnode* next[];
};

#define SKIP_LIST_MAX_LEVEL 32
#define SKIP_LIST_P 0.25

struct skiplist {
    struct slnode* head;
    struct slnode* tail;
    long length;
    int level;
    COMPARE compare;
};

SKIPLIST* open_skiplist(COMPARE compare) {
    struct skiplist* ret = NEW(struct skiplist);
    assert(ret != NULL);
    ret->head = NEW2(struct slnode, sizeof(struct slnode*)*SKIP_LIST_MAX_LEVEL);
    memset(ret->head, 0, sizeof(struct slnode)+sizeof(struct slnode*)*SKIP_LIST_MAX_LEVEL);
    ret->tail = ret->head;
    ret->length = 0;
    ret->level = 1;
    ret->compare = compare;

    static int init_once = 1;
    if (init_once) {
        srand((unsigned int)time(0));
        init_once = 0;
    }

    return ret;
}

void close_skiplist(SKIPLIST* sl) {
    assert(sl != NULL);
    skiplist_clear(sl);
    DELETE(sl->head);
    DELETE(sl);
}

void skiplist_clear(SKIPLIST* sl) {
    assert(sl != NULL);
    struct slnode* next;
    for (struct slnode* i=sl->head->next[0]; i!=NULL; i=next) {
        next = i->next[0];
        DELETE(i);
    }
    sl->length = 0;
    sl->level = 1;
    memset(sl->head, 0, sizeof(struct slnode)+sizeof(struct slnode*)*SKIP_LIST_MAX_LEVEL);
    sl->tail = sl->head;
}

static inline int skiplist_random_level() {
//    static int i = 0;
//    int dbg_level[] = {1, 2, 4, 1, 3, 1, 1};
//    return dbg_level[i++];
    int level;
    for (level=1; rand()%100<100*SKIP_LIST_P && level<SKIP_LIST_MAX_LEVEL; ++level);
    return level;
}

/*
 * return prev
 */
struct slnode* skiplist_fast_get(SKIPLIST* sl, VALUE key) {
    assert(sl != NULL);
    register COMPARE compare = sl->compare;
    struct slnode* ret = sl->head;
    for (int level=sl->level-1; level>=0; --level) {
        for (register struct slnode* next=ret->next[level]; next!=NULL; next=ret->next[level]) {
            if (compare(next->value, key) > 0) {
                break;
            }
            ret = next;
        }
    }
    return ret;
}

void skiplist_set(SKIPLIST* sl, VALUE value) {
    assert(sl != NULL);
    register COMPARE compare = sl->compare;
    struct slnode* prev[SKIP_LIST_MAX_LEVEL+1] = {};
    prev[sl->level] = sl->head;
    for (int l=sl->level-1; l>=0; --l) {
        prev[l] = prev[l+1];
        for (register struct slnode* next=prev[l]->next[l]; next!=NULL; next=prev[l]->next[l]) {
            int cmp = compare(next->value, value);
            if (cmp < 0) {
                prev[l] = next;
            }
            else if (cmp > 0) {
                break;
            } else {
                next->value = value;
                return;
            }
        }
    }

    int level = skiplist_random_level();
    struct slnode* node = NEW2(struct slnode, sizeof(struct slnode*)*level);
    node->value = value;

    // link prev
    node->prev = prev[0];
    if (prev[0]->next[0] != NULL) {
        prev[0]->next[0]->prev = node;
    } else {
        sl->tail = node;
    }

    // link next
    if (level <= sl->level) {
        for (int l=0; l<level; ++l) {
            node->next[l] = prev[l]->next[l];
            prev[l]->next[l] = node;
        }
    } else {
        for (int l=0; l<sl->level; ++l) {
            node->next[l] = prev[l]->next[l];
            prev[l]->next[l] = node;
        }
        for (int l=sl->level; l<level; ++l) {
            node->next[l] = NULL;
            sl->head->next[l] = node;
        }
        sl->level = level;
    }
}

VALUE skiplist_get(SKIPLIST* sl, VALUE key, int* ok) {
    assert(sl != NULL);
    register COMPARE compare = sl->compare;
    struct slnode* prev = sl->head;
    for (int l=sl->level-1; l>=0; --l) {
        for (register struct slnode* next=prev->next[l]; next!=NULL; next=prev->next[l]) {
            int cmp = compare(next->value, key);
            if (cmp < 0) {
                prev = next;
            }
            else if (cmp > 0) {
                break;
            } else {
                if (ok) {
                    *ok = 1;
                }
                return next->value;
            }
        }
    }
    if (ok) {
        *ok = 0;
    }
    return NULL_VALUE;
}

void skiplist_test() {
    printf("=== skiplist test ===\n");
    SKIPLIST* sl = open_skiplist(asc_order_int);
    skiplist_set(sl, int_value(22));
    skiplist_set(sl, int_value(19));
    skiplist_set(sl, int_value(7));
    skiplist_set(sl, int_value(3));
    skiplist_set(sl, int_value(37));
    skiplist_set(sl, int_value(11));
    skiplist_set(sl, int_value(26));

    for (int i=0; i<1000; ++i) {
        skiplist_set(sl, int_value(rand()%10000));
    }

    for (int l=sl->level-1; l>=0; --l) {
        for (struct slnode* next=sl->head->next[l]; next!=NULL; next=next->next[l]) {
            printf("%ld ", next->value.int_value);
        }
        printf("\n");
    }
    for (struct slnode* last=sl->tail; last!=sl->head; last=last->prev) {
        printf("%ld ", last->value.int_value);
    }
    printf("\n");

    int ok;
    VALUE res = skiplist_get(sl, int_value(370), &ok);
    printf("get: %d %ld\n", ok, res.int_value);

    close_skiplist(sl);
    printf("\n");
}
