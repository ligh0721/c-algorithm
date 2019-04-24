//
// Created by t5w0rd on 19-4-22.
//

#ifndef ALGORITHM_RBTREE_TPL_H
#define ALGORITHM_RBTREE_TPL_H

#include <assert.h>
#include <math.h>
#include "stack.h"


#define TPL_RB_RED      0
#define TPL_RB_BLACK    1

#define RBTREE_DECLARE(ValueType) \
typedef struct ValueType##_rbtree ValueType##_RBTREE;\
\
ValueType##_RBTREE* open_##ValueType##_rbtree(ValueType##_COMPARE compare);\
ValueType##_RBTREE* open_##ValueType##_rbtree_with_allocator(ValueType##_COMPARE compare, ALLOCATOR allocator);\
void close_##ValueType##_rbtree(ValueType##_RBTREE* tr);\
long ValueType##_rbtree_len(ValueType##_RBTREE* tr);\
void ValueType##_rbtree_clear(ValueType##_RBTREE* tr);\
\
ValueType ValueType##_rbtree_get(ValueType##_RBTREE* tr, ValueType key, int* ok);\
void ValueType##_rbtree_set(ValueType##_RBTREE *tr, ValueType value);\
ValueType ValueType##_rbtree_pop(ValueType##_RBTREE* tr, ValueType key, int* ok);\
\
void ValueType##_rbtree_ldr(ValueType##_RBTREE* tr, ValueType##_TRAVERSE traverse, void* param);\
\
typedef struct ValueType##_rbnode ValueType##_RBNODE;\
ValueType##_RBNODE* ValueType##_rbtree_open_node(ValueType##_RBTREE* tr, ValueType value, ValueType##_RBNODE* parent);\
void ValueType##_rbtree_close_node(ValueType##_RBTREE* tr, ValueType##_RBNODE* node);\
ValueType* ValueType##_rbtree_fast_value(ValueType##_RBTREE* tr, ValueType##_RBNODE** where);\
ValueType##_RBNODE** ValueType##_rbtree_fast_get(ValueType##_RBTREE *tr, ValueType key, ValueType##_RBNODE** parent);\
void ValueType##_rbtree_fast_set(ValueType##_RBTREE *tr, ValueType##_RBNODE** where, ValueType##_RBNODE* node);\
ValueType ValueType##_rbtree_fast_pop(ValueType##_RBTREE *tr, ValueType##_RBNODE *node);\
int ValueType##_rbtree_node_not_found(ValueType##_RBTREE* tr, ValueType##_RBNODE** where);

#define RBTREE_DEFINE(ValueType) \
struct ValueType##_rbnode {\
    ValueType value;\
    union {\
        unsigned long parent_and_color;\
        struct {\
            unsigned long color:1;\
        };\
    };\
    struct ValueType##_rbnode* right;\
    struct ValueType##_rbnode* left;\
} __attribute__((aligned(sizeof(long))));\
\
static inline struct ValueType##_rbnode* ValueType##_rbnode_parent(struct ValueType##_rbnode* node) {\
    return (struct ValueType##_rbnode*)((node)->parent_and_color & ~1);\
}\
\
static inline int ValueType##_rbnode_is_red(struct ValueType##_rbnode* node) {\
    return !(node->parent_and_color & 1);\
}\
\
static inline int ValueType##_rbnode_is_black(struct ValueType##_rbnode* node) {\
    return node->parent_and_color & 1;\
}\
\
static inline void ValueType##_rbnode_set_red(struct ValueType##_rbnode* node) {\
    node->parent_and_color &= ~1;\
}\
\
static inline void ValueType##_rbnode_set_black(struct ValueType##_rbnode* node) {\
    node->parent_and_color |= 1;\
}\
\
static inline void ValueType##_rbnode_set_parent(struct ValueType##_rbnode *node, struct ValueType##_rbnode *parent) {\
    node->parent_and_color = (node->parent_and_color & 1) | (unsigned long)parent;\
}\
\
static inline void ValueType##_rbnode_copy_color(struct ValueType##_rbnode *node, struct ValueType##_rbnode *src) {\
    node->parent_and_color = (node->parent_and_color & ~1) | (src->parent_and_color & 1);\
}\
\
\
struct ValueType##_rbtree {\
    struct ValueType##_rbnode* root;\
    struct ValueType##_rbnode leaf;\
    ValueType##_COMPARE compare;\
    long length;\
    ALLOCATOR allocator;\
};\
\
static inline void* ValueType##_rbtree_new(ValueType##_RBTREE* tr, size_t size) {\
    return tr->allocator.alloc != NULL ? tr->allocator.alloc(size) : NEW0(size);\
}\
\
static inline void ValueType##_rbtree_delete(ValueType##_RBTREE* tr, void* p) {\
    if (tr->allocator.free != NULL) {\
        tr->allocator.free(p);\
    } else if (tr->allocator.alloc == NULL) {\
        DELETE(p);\
    }\
}\
\
ValueType##_RBTREE* open_##ValueType##_rbtree(ValueType##_COMPARE compare) {\
    struct ValueType##_rbtree* ret = NEW(struct ValueType##_rbtree);\
    assert(ret != NULL);\
    ret->leaf.value = ValueType##_EMPTY;\
    ret->leaf.parent_and_color = TPL_RB_BLACK;\
    ret->leaf.left = ret->leaf.right = NULL;\
    ret->root = &ret->leaf;\
    ret->compare = compare;\
    ret->length = 0;\
    ret->allocator = NULL_ALLOCATOR;\
    return ret;\
}\
\
ValueType##_RBTREE* open_##ValueType##_rbtree_with_allocator(ValueType##_COMPARE compare, ALLOCATOR allocator) {\
    assert(allocator.alloc != NULL);\
    struct ValueType##_rbtree* ret = (struct ValueType##_rbtree*)allocator.alloc(sizeof(struct ValueType##_rbtree));\
    assert(ret != NULL);\
    ret->leaf.value = ValueType##_EMPTY;\
    ret->leaf.parent_and_color = TPL_RB_BLACK;\
    ret->leaf.left = ret->leaf.right = NULL;\
    ret->root = &ret->leaf;\
    ret->compare = compare;\
    ret->length = 0;\
    ret->allocator = allocator;\
    return ret;\
}\
\
void close_##ValueType##_rbtree(ValueType##_RBTREE* tr) {\
    assert(tr != NULL);\
    ValueType##_rbtree_clear(tr);\
    ValueType##_rbtree_delete(tr, tr);\
}\
\
long ValueType##_rbtree_len(ValueType##_RBTREE* tr) {\
    assert(tr != NULL);\
    return tr->length;\
}\
\
static inline int ValueType##_rbtree_empty(ValueType##_RBTREE* tr) {\
    return tr->root == &tr->leaf;\
}\
\
void ValueType##_rbtree_clear(ValueType##_RBTREE* tr) {\
    assert(tr != NULL);\
    if (ValueType##_rbtree_empty(tr)) {\
        return;\
    }\
    STACK* st = open_stack(log2(ValueType##_rbtree_len(tr)+1)*2);\
    struct ValueType##_rbnode* top = tr->root;\
    struct ValueType##_rbnode* right;\
    stack_push(st, ptr_value(top));\
    while (stack_len(st) > 0) {\
        while (top->left != &tr->leaf) {\
            top = top->left;\
            stack_push(st, ptr_value(top));\
        }\
        do {\
            top = (struct ValueType##_rbnode*)stack_pop(st, NULL).ptr_value;\
            right = top->right;\
            ValueType##_rbtree_close_node(tr, top);\
        } while (right == &tr->leaf && stack_len(st) > 0);\
        if (right != &tr->leaf) {\
            top = right;\
            stack_push(st, ptr_value(top));\
        }\
    }\
    close_stack(st);\
    tr->root = &tr->leaf;\
    tr->length = 0;\
}\
\
inline ValueType##_RBNODE* ValueType##_rbtree_open_node(ValueType##_RBTREE* tr, ValueType value, ValueType##_RBNODE* parent) {\
    struct ValueType##_rbnode* ret = (struct ValueType##_rbnode*)ValueType##_rbtree_new(tr, sizeof(struct ValueType##_rbnode));\
    ret->value = value;\
    ret->parent_and_color = (unsigned long)parent;\
    ret->left = ret->right = &tr->leaf;\
    return ret;\
}\
\
inline void ValueType##_rbtree_close_node(ValueType##_RBTREE* tr, ValueType##_RBNODE* node) {\
    ValueType##_rbtree_delete(tr, node);\
}\
\
inline ValueType* ValueType##_rbtree_fast_value(ValueType##_RBTREE* tr, ValueType##_RBNODE** where) {\
    return &(*where)->value;\
}\
\
static inline void ValueType##_rbtree_left_rotate(ValueType##_RBTREE* tr, struct ValueType##_rbnode* node) {\
    assert(node->right != &tr->leaf);\
    struct ValueType##_rbnode* right = node->right;\
    struct ValueType##_rbnode* parent = ValueType##_rbnode_parent(node);\
\
    node->right = right->left;\
    if (right->left != &tr->leaf) {\
        ValueType##_rbnode_set_parent(right->left, node);\
    }\
\
    ValueType##_rbnode_set_parent(right, parent);\
    if (node == tr->root) {\
        tr->root = right;\
    } else if (node == parent->left) {\
        parent->left = right;\
    } else {\
        parent->right = right;\
    }\
    right->left = node;\
    ValueType##_rbnode_set_parent(node, right);\
}\
\
static inline void ValueType##_rbtree_right_rotate(ValueType##_RBTREE* tr, struct ValueType##_rbnode* node) {\
    assert(node->left != &tr->leaf);\
    struct ValueType##_rbnode* left = node->left;\
    struct ValueType##_rbnode* parent = ValueType##_rbnode_parent(node);\
\
    node->left = left->right;\
    if (left->right != &tr->leaf) {\
        ValueType##_rbnode_set_parent(left->right, node);\
    }\
\
    ValueType##_rbnode_set_parent(left, parent);\
    if (node == tr->root) {\
        tr->root = left;\
    } else if (node == parent->right) {\
        parent->right = left;\
    } else {\
        parent->left = left;\
    }\
    left->right = node;\
    ValueType##_rbnode_set_parent(node, left);\
}\
/*\
 *  if *ret == leaf, not found (ret == &root)\
 *  else found\
 */\
inline ValueType##_RBNODE** ValueType##_rbtree_fast_get(ValueType##_RBTREE *tr, const ValueType key, ValueType##_RBNODE** parent) {\
    struct ValueType##_rbnode** ret = &tr->root;\
    struct ValueType##_rbnode* leaf = &tr->leaf;\
    if (*ret == leaf) {\
        if (parent) {\
            *parent = NULL;\
        }\
        return ret;\
    }\
\
    register ValueType##_COMPARE compare = tr->compare;\
    for (;;) {\
        struct ValueType##_rbnode* node = *ret;\
        register int cmp = compare(key, node->value);\
        if (cmp == 0) {\
            if (parent) {\
                *parent = ValueType##_rbnode_parent(node);\
            }\
            return ret;\
        }\
        ret = cmp < 0 ? &node->left : &node->right;\
        if (*ret == leaf) {\
            if (parent) {\
                *parent = node;\
            }\
            return ret;\
        }\
    }\
}\
\
void ValueType##_rbtree_fast_set(ValueType##_RBTREE *tr, ValueType##_RBNODE** where, ValueType##_RBNODE* node) {\
    assert(tr != NULL);\
    assert(*where == &tr->leaf);\
    *where = node;\
    tr->length++;\
    if (where == &tr->root) {\
        ValueType##_rbnode_set_black(tr->root);\
        return;\
    }\
\
    while (node != tr->root && ValueType##_rbnode_is_red(ValueType##_rbnode_parent(node))) {\
        if (ValueType##_rbnode_parent(node) == ValueType##_rbnode_parent(ValueType##_rbnode_parent(node))->left) {\
            struct ValueType##_rbnode* uncle = ValueType##_rbnode_parent(ValueType##_rbnode_parent(node))->right;\
            if (ValueType##_rbnode_is_red(uncle)) {\
                ValueType##_rbnode_set_black(ValueType##_rbnode_parent(node));\
                ValueType##_rbnode_set_black(uncle);\
                ValueType##_rbnode_set_red(ValueType##_rbnode_parent(ValueType##_rbnode_parent(node)));\
                node = ValueType##_rbnode_parent(ValueType##_rbnode_parent(node));\
            } else {\
                if (node == ValueType##_rbnode_parent(node)->right) {\
                    node = ValueType##_rbnode_parent(node);\
                    ValueType##_rbtree_left_rotate(tr, node);\
                }\
\
                ValueType##_rbnode_set_black(ValueType##_rbnode_parent(node));\
                ValueType##_rbnode_set_red(ValueType##_rbnode_parent(ValueType##_rbnode_parent(node)));\
                ValueType##_rbtree_right_rotate(tr, ValueType##_rbnode_parent(ValueType##_rbnode_parent(node)));\
            }\
        } else {\
            struct ValueType##_rbnode* uncle = ValueType##_rbnode_parent(ValueType##_rbnode_parent(node))->left;\
            if (ValueType##_rbnode_is_red(uncle)) {\
                ValueType##_rbnode_set_black(ValueType##_rbnode_parent(node));\
                ValueType##_rbnode_set_black(uncle);\
                ValueType##_rbnode_set_red(ValueType##_rbnode_parent(ValueType##_rbnode_parent(node)));\
                node = ValueType##_rbnode_parent(ValueType##_rbnode_parent(node));\
            } else {\
                if (node == ValueType##_rbnode_parent(node)->left) {\
                    node = ValueType##_rbnode_parent(node);\
                    ValueType##_rbtree_right_rotate(tr, node);\
                }\
\
                ValueType##_rbnode_set_black(ValueType##_rbnode_parent(node));\
                ValueType##_rbnode_set_red(ValueType##_rbnode_parent(ValueType##_rbnode_parent(node)));\
                ValueType##_rbtree_left_rotate(tr, ValueType##_rbnode_parent(ValueType##_rbnode_parent(node)));\
            }\
        }\
    }\
    ValueType##_rbnode_set_black(tr->root);\
}\
\
ValueType ValueType##_rbtree_fast_pop(ValueType##_RBTREE *tr, ValueType##_RBNODE *node) {\
    assert(tr != NULL);\
    assert(node != NULL);\
    ValueType ret = node->value;\
    struct ValueType##_rbnode* leaf = &tr->leaf;\
    struct ValueType##_rbnode* temp;\
    struct ValueType##_rbnode* subst;\
\
    if (node->left == leaf) {\
        temp = node->right;\
        subst = node;\
    } else if (node->right == leaf) {\
        temp = node->left;\
        subst = node;\
    } else {\
        for (subst=node->right; subst->left!=leaf; subst=subst->left);\
        if (subst->left != leaf) {\
            temp = subst->left;\
        } else {\
            temp = subst->right;\
        }\
    }\
\
    if (subst == tr->root) {\
        tr->root = temp;\
        ValueType##_rbnode_set_black(temp);\
        tr->length--;\
        return ret;\
    }\
\
    unsigned long red = ValueType##_rbnode_is_red(subst);\
\
    if (subst == ValueType##_rbnode_parent(subst)->left) {\
        ValueType##_rbnode_parent(subst)->left = temp;\
    } else {\
        ValueType##_rbnode_parent(subst)->right = temp;\
    }\
\
    if (subst == node) {\
        ValueType##_rbnode_set_parent(temp, ValueType##_rbnode_parent(subst));\
    } else {\
        if (ValueType##_rbnode_parent(subst) == node) {\
            ValueType##_rbnode_set_parent(temp, subst);\
        } else {\
            ValueType##_rbnode_set_parent(temp, ValueType##_rbnode_parent(subst));\
        }\
\
        subst->left = node->left;\
        subst->right = node->right;\
        subst->parent_and_color = node->parent_and_color;\
\
        if (node == tr->root) {\
            tr->root = subst;\
        } else {\
            if (node == ValueType##_rbnode_parent(node)->left) {\
                ValueType##_rbnode_parent(node)->left = subst;\
            } else {\
                ValueType##_rbnode_parent(node)->right = subst;\
            }\
        }\
\
        if (subst->left != leaf) {\
            ValueType##_rbnode_set_parent(subst->left, subst);\
        }\
\
        if (subst->right != leaf) {\
            ValueType##_rbnode_set_parent(subst->right, subst);\
        }\
    }\
\
    tr->length--;\
\
    if (red) {\
        return ret;\
    }\
\
    struct ValueType##_rbnode* w;\
    while (temp != tr->root && ValueType##_rbnode_is_black(temp)) {\
        if (temp == ValueType##_rbnode_parent(temp)->left) {\
            w = ValueType##_rbnode_parent(temp)->right;\
\
            if (ValueType##_rbnode_is_red(w)) {\
                ValueType##_rbnode_set_black(w);\
                ValueType##_rbnode_set_red(ValueType##_rbnode_parent(temp));\
                ValueType##_rbtree_left_rotate(tr, ValueType##_rbnode_parent(temp));\
                w = ValueType##_rbnode_parent(temp)->right;\
            }\
\
            if (ValueType##_rbnode_is_black(w->left) && ValueType##_rbnode_is_black(w->right)) {\
                ValueType##_rbnode_set_red(w);\
                temp = ValueType##_rbnode_parent(temp);\
            } else {\
                if (ValueType##_rbnode_is_black(w->right)) {\
                    ValueType##_rbnode_set_black(w->left);\
                    ValueType##_rbnode_set_red(w);\
                    ValueType##_rbtree_right_rotate(tr, w);\
                    w = ValueType##_rbnode_parent(temp)->right;\
                }\
\
                ValueType##_rbnode_copy_color(w, ValueType##_rbnode_parent(temp));\
                ValueType##_rbnode_set_black(ValueType##_rbnode_parent(temp));\
                ValueType##_rbnode_set_black(w->right);\
                ValueType##_rbtree_left_rotate(tr, ValueType##_rbnode_parent(temp));\
                temp = tr->root;\
            }\
\
        } else {\
            w = ValueType##_rbnode_parent(temp)->left;\
\
            if (ValueType##_rbnode_is_red(w)) {\
                ValueType##_rbnode_set_black(w);\
                ValueType##_rbnode_set_red(ValueType##_rbnode_parent(temp));\
                ValueType##_rbtree_right_rotate(tr, ValueType##_rbnode_parent(temp));\
                w = ValueType##_rbnode_parent(temp)->left;\
            }\
\
            if (ValueType##_rbnode_is_black(w->left) && ValueType##_rbnode_is_black(w->right)) {\
                ValueType##_rbnode_set_red(w);\
                temp = ValueType##_rbnode_parent(temp);\
            } else {\
                if (ValueType##_rbnode_is_black(w->left)) {\
                    ValueType##_rbnode_set_black(w->right);\
                    ValueType##_rbnode_set_red(w);\
                    ValueType##_rbtree_left_rotate(tr, w);\
                    w = ValueType##_rbnode_parent(temp)->left;\
                }\
\
                ValueType##_rbnode_copy_color(w, ValueType##_rbnode_parent(temp));\
                ValueType##_rbnode_set_black(ValueType##_rbnode_parent(temp));\
                ValueType##_rbnode_set_black(w->left);\
                ValueType##_rbtree_right_rotate(tr, ValueType##_rbnode_parent(temp));\
                temp = tr->root;\
            }\
        }\
    }\
\
    ValueType##_rbnode_set_black(temp);\
    return ret;\
}\
\
int ValueType##_rbtree_node_not_found(ValueType##_RBTREE* tr, ValueType##_RBNODE** where) {\
    assert(tr != NULL);\
    return *where == &tr->leaf;\
}\
\
ValueType ValueType##_rbtree_get(ValueType##_RBTREE* tr, const ValueType key, int* ok) {\
    ValueType##_RBNODE* where = *ValueType##_rbtree_fast_get(tr, key, NULL);\
    if (where == &tr->leaf) {\
        if (ok) {\
            *ok = 0;\
        }\
        return ValueType##_EMPTY;\
    }\
    if (ok) {\
        *ok = 1;\
    }\
    return where->value;\
}\
\
void ValueType##_rbtree_set(ValueType##_RBTREE *tr, ValueType value) {\
    assert(tr != NULL);\
    ValueType##_RBNODE* parent;\
    ValueType##_RBNODE** where = ValueType##_rbtree_fast_get(tr, value, &parent);\
    if (*where != &tr->leaf) {\
        (*where)->value = value;\
        return;\
    }\
    struct ValueType##_rbnode* node = ValueType##_rbtree_open_node(tr, value, parent);\
    ValueType##_rbtree_fast_set(tr, where, node);\
}\
\
ValueType ValueType##_rbtree_pop(ValueType##_RBTREE* tr, const ValueType key, int* ok) {\
    assert(tr != NULL);\
    ValueType##_RBNODE* search = *ValueType##_rbtree_fast_get(tr, key, NULL);\
    if (search == NULL || search == &tr->leaf) {\
        if (ok) {\
            *ok = 0;\
        }\
        return ValueType##_EMPTY;\
    }\
    if (ok) {\
        *ok = 1;\
    }\
    ValueType ret = ValueType##_rbtree_fast_pop(tr, search);\
    ValueType##_rbtree_close_node(tr, search);\
    return ret;\
}\
\
void ValueType##_rbtree_ldr(ValueType##_RBTREE* tr, ValueType##_TRAVERSE traverse, void* param) {\
    assert(tr != NULL);\
    if (ValueType##_rbtree_empty(tr)) {\
        return;\
    }\
    STACK* st = open_stack(log2(ValueType##_rbtree_len(tr)+1)*2);\
    struct ValueType##_rbnode* top = tr->root;\
    stack_push(st, ptr_value(top));\
    while (stack_len(st) > 0) {\
        while (top->left != &tr->leaf) {\
            top = top->left;\
            stack_push(st, ptr_value(top));\
        }\
        do {\
            top = (struct ValueType##_rbnode*)stack_pop(st, NULL).ptr_value;\
            if (traverse(&top->value, param)) {\
                close_stack(st);\
                return;\
            }\
        } while (top->right == &tr->leaf && stack_len(st) > 0);\
        if (top->right != &tr->leaf) {\
            top = top->right;\
            stack_push(st, ptr_value(top));\
        }\
    }\
    close_stack(st);\
}


#endif //ALGORITHM_RBTREE_TPL_H
