//
// Created by t5w0rd on 2019-04-16.
//

#include <assert.h>
#include "rbtree.h"


struct rb_node {
    VALUE value;
    unsigned long parent_and_color;  // 父节点的指针以及颜色
#define RB_RED      0
#define RB_BLACK    1
    struct rb_node* right;
    struct rb_node* left;
} __attribute__((aligned(sizeof(long))));


static inline struct rb_node* open_rb_node(const VALUE value) {
    struct rb_node* ret = NEW(struct rb_node);
    ret->value = value;
    ret->left = ret->right = NULL;
    return ret;
}

static inline void close_rb_node(struct rb_node* node) {
    DELETE(node);
}


#define rb_node_parent(node) ((struct rb_node*)((node)->parent_and_color & ~3))
#define rb_node_color(node) ((node)->parent_and_color & 1)
#define rb_node_is_red(node) (!rb_node_color(node))
#define rb_node_is_black(node) rb_node_color(node)
#define rb_node_set_red(node) do { (node)->parent_and_color &= ~1; } while (0)
#define rb_node_set_black(node) do { (node)->parent_and_color |= 1; } while (0)
#define rb_node_empty(node) (rb_node_parent(node) == node)
#define rb_node_clear(node) (rb_set_parent(node, node))

// 只设置父节点地址
static inline void rb_node_set_parent(struct rb_node *node, struct rb_node *parent) {
    node->parent_and_color = (node->parent_and_color & 3) | (unsigned long)parent;
}

// 只设置父节点颜色
static inline void rb_set_color(struct rb_node* node, int color) {
    node->parent_and_color = (node->parent_and_color & ~1) | color;
}

static inline void rb_node_set_parent_and_color(struct rb_node *node, struct rb_node *parent, int color) {
    node->parent_and_color = (node->parent_and_color & ~1) | color | (unsigned long)parent;
}

static inline void rb_node_link_node(struct rb_node *node, struct rb_node *parent, struct rb_node **link) {
    node->parent_and_color = (unsigned long)parent;
    node->left = node->right = NULL;
    *link = node;
}


struct rbtree {
    struct rb_node* root;
    struct rb_node* leaf;
    COMPARE compare;
};

RBTREE* open_rbtree(COMPARE compare) {
    struct rbtree* ret = NEW(struct rbtree);
    assert(ret != NULL);
    ret->root = NULL;
    ret->leaf = open_rb_node(NULL_VALUE);
    ret->leaf->parent_and_color = 0;
    ret->compare = compare;
    return ret;
}

void close_rbtree(RBTREE* tr) {
    assert(tr != NULL);
    rbtree_clear(tr);
    close_rb_node(tr->leaf);
    DELETE(tr);
}

void rbtree_clear(RBTREE* tr) {
    assert(tr != NULL);
}

// 默认红色
static inline struct rb_node* rbtree_open_node(RBTREE* tr, const VALUE value, struct rb_node* parent) {
    struct rb_node* ret = NEW(struct rb_node);
    ret->value = value;
    ret->parent_and_color = (unsigned long)parent;
    ret->left = ret->right = tr->leaf;
    return ret;
}

#define rbtree_empty(rbtree) ((rbtree)->root == NULL)

void rbtree_insert(RBTREE* tr, const VALUE value) {
    assert(tr != NULL);
    if (rbtree_empty(tr)) {
        tr->root = rbtree_open_node(tr, value, NULL);
        rb_node_set_black(tr->root);
        return;
    }

    // insert node as a Binary Search Tree
    struct rb_node* leaf = tr->leaf;
    struct rb_node* node = tr->root;
    for (;;) {
        register int cmp = tr->compare(value, node->value);
        if (cmp == 0) {
            node->value = value;
            return;
        }
        if (cmp < 0) {
            if (node->left == leaf) {
                node->left = rbtree_open_node(tr, value, node);
                break;
            }
            node = node->left;
        } else {
            if (node->right == leaf) {
                node->right = rbtree_open_node(tr, value, node);
                break;
            }
            node = node->right;
        }
    }
}
#include "stack.h"
#include <stdio.h>

void rbtree_ldr(RBTREE* tr) {
    assert(tr != NULL);
    if (rbtree_empty(tr)) {
        return;
    }
    STACK* st = open_stack(100);
    struct rb_node* top = tr->root;
    stack_push(st, ptr_value(top));
    while (stack_len(st) > 0) {
        while (top->left != tr->leaf) {
            top = top->left;
            stack_push(st, ptr_value(top));
        }
        do {
            top = (struct rb_node*)stack_pop(st, NULL).ptr_value;
            printf("%ld ", top->value.int_value);
        } while (top->right == tr->leaf && stack_len(st) > 0);
        if (top->right != tr->leaf) {
            top = top->right;
            stack_push(st, ptr_value(top));
        }
    }
    printf("\n");
    close_stack(st);
}

void rbtree_test() {
    RBTREE* tr = open_rbtree(asc_order_int);
    rbtree_insert(tr, int_value(5));
    rbtree_insert(tr, int_value(5));
    rbtree_insert(tr, int_value(8));
    rbtree_insert(tr, int_value(3));
    rbtree_insert(tr, int_value(1));
    rbtree_insert(tr, int_value(-6));
    rbtree_insert(tr, int_value(7));
    rbtree_ldr(tr);
    close_rbtree(tr);
}