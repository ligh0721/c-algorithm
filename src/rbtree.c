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

#define rb_parent(node) ((struct rb_node*)((node)->parent_and_color & ~3))
#define rb_color(node) ((node)->parent_and_color & 1)
#define rb_is_red(node) (!rb_color(node))
#define rb_is_black(node) rb_color(node)
#define rb_set_red(node) do { ((node)->parent_and_color &= ~1; } while (0)
#define rb_set_black(node) do { ((node)->parent_and_color |= 1; } while (0)
#define rb_empty(node) (rb_parent(node) == node)
#define rb_clear(node) (rb_set_parent(node, node))

// 只设置父节点地址
static inline void rb_set_parent(struct rb_node* node, struct rb_node* parent) {
    node->parent_and_color = (node->parent_and_color & 3) | (unsigned long)parent;
}

// 只设置父节点颜色
static inline void rb_set_color(struct rb_node* node, int color) {
    node->parent_and_color = (node->parent_and_color & ~1) | color;
}

static inline void rb_link_node(struct rb_node* node, struct rb_node* parent, struct rb_node** link) {
    node->parent_and_color = (unsigned long)parent;
    node->left = node->right = NULL;
    *link = node;
}


struct rbtree {
    struct rb_node* root;
};

#define rbtree_empty(rbtree) ((rbtree)->root == NULL)

RBTREE* open_rbtree() {
    struct rbtree* ret = NEW(struct rbtree);
    assert(ret != NULL);
    ret->root = NULL;
    return ret;
}

void close_rbtree(RBTREE* tr) {
    assert(tr != NULL);
    // TODO: DELETE ALL NODE
    DELETE(tr);
}

void rbtree_insert_color(RBTREE* tr, struct rb_node* node) {
    struct rb_node* parent;
    struct rb_node* gparent;
    while ((parent = rb_parent(node)) && rb_is_red(node)) {
        gparent = rb_parent(parent);
        if (parent == gparent->left) {

        } else {

        }
    }
}

void rbtree_test() {
}