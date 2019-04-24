//
// Created by t5w0rd on 2019-04-24.
//

#ifndef ALGORITHM_LINK_TPL_H
#define ALGORITHM_LINK_TPL_H


#define LLINK_DECLARE(ValueType) \
struct ValueType##_lnode {\
    ValueType value;\
    struct ValueType##_lnode* next;\
};\
\
typedef struct ValueType##_llist ValueType##_LLIST;\
\
ValueType##_LLIST* open_##ValueType##_llist();\
ValueType##_LLIST* open_##ValueType##_llist_with_allocator(ALLOCATOR allocator);\
void close_##ValueType##_llist(ValueType##_LLIST *lst);\
void ValueType##_llist_clear(ValueType##_LLIST* lst);\
long ValueType##_llist_len(ValueType##_LLIST* lst);\
struct ValueType##_lnode* ValueType##_llist_front_node(ValueType##_LLIST* lst);\
struct ValueType##_lnode* ValueType##_llist_back_node(ValueType##_LLIST* lst);\
struct ValueType##_lnode* ValueType##_llist_before_front_node(ValueType##_LLIST *lst);\
int ValueType##_llist_is_node_before_front(ValueType##_LLIST *lst, struct ValueType##_lnode *back);\
void ValueType##_llist_traversal(ValueType##_LLIST* lst, ValueType##_TRAVERSE traverse, void* param);\
void ValueType##_llist_push_back(ValueType##_LLIST* lst, ValueType value);\

#define LLINK_DEFINE(ValueType) \
struct ValueType##_llist {\
    struct ValueType##_lnode head;\
    struct ValueType##_lnode* tail;\
    long length;\
    ALLOCATOR allocator;\
};\
\
static inline void* ValueType##_llist_new(ValueType##_LLIST* lst, size_t size) {\
    return lst->allocator.alloc != NULL ? lst->allocator.alloc(size) : NEW0(size);\
}\
\
static inline void ValueType##_llist_delete(ValueType##_LLIST* lst, void* p) {\
    if (lst->allocator.free != NULL) {\
        lst->allocator.free(p);\
    } else if (lst->allocator.alloc == NULL) {\
        DELETE(p);\
    }\
}\
\
ValueType##_LLIST* open_##ValueType##_llist() {\
    struct ValueType##_llist* ret = NEW(struct ValueType##_llist);\
    assert(ret != NULL);\
    ret->head.value = ValueType##_EMPTY;\
    ret->head.next = NULL;\
    ret->tail = &ret->head;\
    ret->length = 0;\
    ret->allocator = NULL_ALLOCATOR;\
    return ret;\
}\
\
ValueType##_LLIST* open_##ValueType##_llist_with_allocator(ALLOCATOR allocator) {\
    assert(allocator.alloc != NULL);\
    struct ValueType##_llist* ret = (struct ValueType##_llist*)allocator.alloc(sizeof(struct ValueType##_llist));\
    assert(ret != NULL);\
    ret->head.value = ValueType##_EMPTY;\
    ret->head.next = NULL;\
    ret->tail = &ret->head;\
    ret->length = 0;\
    ret->allocator = allocator;\
    return ret;\
}\
\
void close_##ValueType##_llist(ValueType##_LLIST *lst) {\
    assert(lst != NULL);\
    ValueType##_llist_clear(lst);\
    ValueType##_llist_delete(lst, lst);\
}\
\
void ValueType##_llist_clear(ValueType##_LLIST* lst) {\
    assert(lst != NULL);\
    for (struct ValueType##_lnode* node=lst->head.next; node!=NULL; ) {\
        struct ValueType##_lnode* del = node;\
        node = node->next;\
        ValueType##_llist_delete(lst, del);\
    }\
    lst->head.next = NULL;\
    lst->tail = &lst->head;\
    lst->length = 0;\
}\
\
long ValueType##_llist_len(ValueType##_LLIST* lst) {\
    assert(lst != NULL);\
    return lst->length;\
}\
\
struct ValueType##_lnode* ValueType##_llist_front_node(ValueType##_LLIST* lst) {\
    assert(lst != NULL);\
    return lst->head.next;\
}\
\
struct ValueType##_lnode* ValueType##_llist_back_node(ValueType##_LLIST* lst) {\
    assert(lst != NULL);\
    return lst->tail;\
}\
\
struct ValueType##_lnode* ValueType##_llist_before_front_node(ValueType##_LLIST *lst) {\
    assert(lst != NULL);\
    return &lst->head;\
}\
\
int ValueType##_llist_is_node_before_front(ValueType##_LLIST *lst, struct ValueType##_lnode *back) {\
    assert(lst != NULL);\
    return lst->tail == &lst->head;\
}\
\
void ValueType##_llist_traversal(ValueType##_LLIST* lst, ValueType##_TRAVERSE traverse, void* param) {\
    assert(lst != NULL);\
    for (struct ValueType##_lnode* node=lst->head.next; node!=NULL; node=node->next) {\
        if (traverse(&node->value, param)) {\
            break;\
        }\
    }\
}\
\
void ValueType##_llist_push_back(ValueType##_LLIST* lst, ValueType value) {\
    assert(lst != NULL);\
    struct ValueType##_lnode* node = (struct ValueType##_lnode*)ValueType##_llist_new(lst, sizeof(struct ValueType##_lnode));\
    node->value = value;\
    node->next = NULL;\
    lst->tail = lst->tail->next = node;\
    lst->length++;\
}

#endif //ALGORITHM_LINK_TPL_H
