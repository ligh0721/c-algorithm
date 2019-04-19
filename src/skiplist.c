//
// Created by t5w0rd on 19-4-18.
//

#include <assert.h>
#include <string.h>
#include <time.h>
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
//    struct slnode* tail;
    long length;
    int level;
    COMPARE compare;
    COMPARE compare2;
};

SKIPLIST* open_skiplist(COMPARE compare, COMPARE compare2) {
    struct skiplist* ret = NEW(struct skiplist);
    assert(ret != NULL);
    ret->head = NEW2(struct slnode, sizeof(struct slnode*)*SKIP_LIST_MAX_LEVEL);
    memset(ret->head, 0, sizeof(struct slnode)+sizeof(struct slnode*)*SKIP_LIST_MAX_LEVEL);
//    ret->tail = ret->head;
    ret->length = 0;
    ret->level = 1;
    ret->compare = compare;
    ret->compare2 = compare2;

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
//    sl->tail = sl->head;
}

long skiplist_len(SKIPLIST* sl) {
    assert(sl != NULL);
    return sl->length;
}

static inline int skiplist_random_level() {
    int level;
    for (level=1; rand()%100<100*SKIP_LIST_P && level<SKIP_LIST_MAX_LEVEL; ++level);
    return level;
}

/*
 * if found return node
 * else
 *     if reverse return prev
 *     else return next
 */
static inline struct slnode* skiplist_fast_get(SKIPLIST* sl, VALUE key, int reverse) {
    assert(sl != NULL);
    register COMPARE compare = sl->compare;
    struct slnode* prev = sl->head;
    register struct slnode* node;
    for (int l=sl->level-1; l>=0; --l) {
        for (node=prev->next[l]; node!=NULL; node=prev->next[l]) {
            int cmp = compare(node->value, key);
            if (cmp < 0) {
                prev = node;
            }
            else if (cmp > 0) {
                break;
            } else {
                return node;
            }
        }
    }
    return reverse ? prev : node;
}

void skiplist_set(SKIPLIST* sl, VALUE value) {
    assert(sl != NULL);
    register COMPARE compare = sl->compare;
    register COMPARE compare2 = sl->compare2;
    struct slnode* prev[SKIP_LIST_MAX_LEVEL+1] = {};
    prev[sl->level] = sl->head;
    for (int l=sl->level-1; l>=0; --l) {
        prev[l] = prev[l+1];
        for (register struct slnode* node=prev[l]->next[l]; node!=NULL; node=prev[l]->next[l]) {
            int cmp = compare(node->value, value);
            if (cmp < 0) {
                prev[l] = node;
            }
            else if (cmp > 0) {
                break;
            } else {
                if (compare2) {
                    cmp = compare2(node->value, value);
                    if (cmp < 0) {
                        prev[l] = node;
                    } else if (cmp > 0) {
                        break;
                    } else {
                        node->value = value;
                        return;
                    }
                } else {
                    node->value = value;
                    return;
                }
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
//        sl->tail = node;
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
    sl->length++;
}

VALUE skiplist_get(SKIPLIST* sl, VALUE key, int* ok) {
    assert(sl != NULL);
    register COMPARE compare = sl->compare;
    struct slnode* prev = sl->head;
    for (int l=sl->level-1; l>=0; --l) {
        for (register struct slnode* node=prev->next[l]; node!=NULL; node=prev->next[l]) {
            int cmp = compare(node->value, key);
            if (cmp < 0) {
                prev = node;
            }
            else if (cmp > 0) {
                break;
            } else {
                if (ok) {
                    *ok = 1;
                }
                return node->value;
            }
        }
    }
    if (ok) {
        *ok = 0;
    }
    return NULL_VALUE;
}

void skiplist_range(SKIPLIST* sl, SLICE* data, VALUE key1, VALUE key2, long limit) {
    assert(sl != NULL);
    register COMPARE compare = sl->compare;
    register long len = 0;
    struct slnode* head = sl->head;
    if (compare(key1, key2) > 0) {
        // reverse, desc order
        for (struct slnode* node=skiplist_fast_get(sl, key1, 1); node!=head && compare(node->value, key2)>=0 && len<limit; node=node->prev) {
            slice_append(data, node->value);
            ++len;
        }
    } else {
        // asc order
        for (struct slnode* node=skiplist_fast_get(sl, key1, 0); node!=NULL && compare(node->value, key2)<=0 && len<limit; node=node->next[0]) {
            slice_append(data, node->value);
            ++len;
        }
    }
}

#include <stdio.h>
void skiplist_debug(SKIPLIST* sl, PRINT_VALUE print) {
    printf("===== skiplist debug =====\n");
    for (int l=sl->level-1; l>=0; --l) {
        printf("L%02d ", l+1);
        for (struct slnode* node=sl->head->next[l]; node!=NULL; node=node->next[l]) {
            print(node->value);
        }
        printf("\n");
    }
    printf("===== len: %ld =====\n", sl->length);
}

long skiplist_remove(SKIPLIST* sl, VALUE key1, VALUE key2) {
    assert(sl != NULL);
    register COMPARE compare = sl->compare;
    if (compare(key1, key2) > 0) {
        VALUE tmp = key1;
        key1 = key2;
        key2 = tmp;
    }
    int level = -1;
    struct slnode* prev[SKIP_LIST_MAX_LEVEL+1] = {};
    prev[sl->level] = sl->head;
    for (int l=sl->level-1; l>=0; --l) {
        prev[l] = prev[l+1];
        for (register struct slnode* node=prev[l]->next[l]; node!=NULL; node=prev[l]->next[l]) {
            int cmp = compare(node->value, key1);
            if (cmp < 0) {
                prev[l] = node;
            } else {
                if (level == -1) {
                    level = l;
                }
                break;
            }
        }
    }

    for (int l=level; l>0; --l) {
        for (struct slnode* pre=prev[l]; pre->next[l]!=NULL && compare(pre->next[l]->value, key2)<=0; ) {
            pre->next[l] = pre->next[l]->next[l];
            if (pre->next[l] == NULL) {
                break;
            }
        }
        if (sl->head->next[l] == NULL && l + 1 == sl->level) {
            sl->level--;
        }
    }
    int ret = 0;
    for (struct slnode* pre=prev[0]; pre->next[0]!=NULL && compare(pre->next[0]->value, key2)<=0; ) {
        struct slnode* remove = pre->next[0];
        if (remove->next[0] != NULL) {
            remove->next[0]->prev = pre;
        } else {
//            sl->tail = prev;
        }
        pre->next[0] = remove->next[0];
        ++ret;
        DELETE(remove);
        if (pre->next[0] == NULL) {
            break;
        }
    }
    sl->length -= ret;
    return ret;
}

