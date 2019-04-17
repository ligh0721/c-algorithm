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

// default red color
static inline struct rb_node* rbtree_open_node(RBTREE* tr, const VALUE value, struct rb_node* parent) {
    struct rb_node* ret = NEW(struct rb_node);
    ret->value = value;
    ret->parent_and_color = (unsigned long)parent;
    ret->left = ret->right = tr->leaf;
    return ret;
}

#define rbtree_empty(rbtree) ((rbtree)->root == NULL)

static inline void rbtree_left_rotate(RBTREE* tr, struct rb_node* node) {
    assert(node->right != tr->leaf);
    struct rb_node* right = node->right;
    struct rb_node* parent = rb_node_parent(node);

    node->right = right->left;
    if (right->left != tr->leaf) {
        rb_node_set_parent(right->left, node);
    }

    rb_node_set_parent(right, parent);
    if (node == tr->root) {
        tr->root = right;
    } else if (node == parent->left) {
        parent->left = right;
    } else {
        parent->right = right;
    }
    right->left = node;
    rb_node_set_parent(node, right);
}

static inline void rbtree_right_rotate(RBTREE* tr, struct rb_node* node) {
    assert(node->left != tr->leaf);
    struct rb_node* left = node->left;
    struct rb_node* parent = rb_node_parent(node);

    node->left = left->right;
    if (left->right != tr->leaf) {
        rb_node_set_parent(left->right, node);
    }

    rb_node_set_parent(left, parent);
    if (node == tr->root) {
        tr->root = left;
    } else if (node == parent->right) {
        parent->right = left;
    } else {
        parent->left = left;
    }
    left->right = node;
    rb_node_set_parent(node, left);
}
/*
 *  if *ret == NULL, not found && empty tree
 *  elif *ret == leaf, not found
 *  else found
 */
static inline struct rb_node** rbtree_search(RBTREE* tr, const VALUE value) {
    struct rb_node** ret = &tr->root;
    if (rbtree_empty(tr)) {
        return ret;
    }
    struct rb_node* leaf = tr->leaf;
    struct rb_node* node = tr->root;
    for (;;) {
        register int cmp = tr->compare(value, node->value);
        if (cmp == 0) {
            return ret;
        }
        ret = cmp < 0 ? &node->left : &node->right;
        if (*ret == leaf) {
            return ret;
        }
        node = *ret;
    }
}

VALUE rbtree_get(RBTREE* tr, const VALUE key, int* ok) {
    if (rbtree_empty(tr)) {
        if (ok) {
            *ok = 0;
        }
        return NULL_VALUE;
    }
    struct rb_node* leaf = tr->leaf;
    struct rb_node* node = tr->root;
    do {
        register int cmp = tr->compare(key, node->value);
        if (cmp == 0) {
            if (ok) {
                *ok = 1;
            }
            return node->value;
        }
        node = cmp < 0 ? node->left : node->right;
    } while (node != leaf);

    if (ok) {
        *ok = 0;
    }
    return NULL_VALUE;
}

#include <stdio.h>
void rbtree_set(RBTREE *tr, const VALUE value) {
    assert(tr != NULL);
    if (rbtree_empty(tr)) {
        tr->root = rbtree_open_node(tr, value, NULL);
        rb_node_set_black(tr->root);
        return;
    }

    // insert node as a Binary Search Tree
    struct rb_node* leaf = tr->leaf;
    struct rb_node* node = tr->root;
    struct rb_node** parent_child;
    for (;;) {
        register int cmp = tr->compare(value, node->value);
        if (cmp == 0) {
            node->value = value;
            return;
        }
        parent_child = cmp < 0 ? &node->left : &node->right;
        if (*parent_child == leaf) {
            *parent_child = node = rbtree_open_node(tr, value, node);
            break;
        }
        node = *parent_child;
    }

    // re-balance tree
    while (node != tr->root && rb_node_is_red(rb_node_parent(node))) {
        // node isnot root and node's parent is red
        if (rb_node_parent(node) == rb_node_parent(rb_node_parent(node))->left) {
            // node's parent is node's grand parent's left child; right uncle
            struct rb_node* uncle = rb_node_parent(rb_node_parent(node))->right;
            if (rb_node_is_red(uncle)) {
                // right uncle is red
                rb_node_set_black(rb_node_parent(node));
                rb_node_set_black(uncle);
                rb_node_set_red(rb_node_parent(rb_node_parent(node)));
                node = rb_node_parent(rb_node_parent(node));
            } else {
                // right uncle is black
                if (node == rb_node_parent(node)->right) {
                    // node is parent's right child
                    node = rb_node_parent(node);
                    rbtree_left_rotate(tr, node);
                }

                // node is parent's left child
                rb_node_is_black(rb_node_parent(node));
                rb_node_is_red(rb_node_parent(rb_node_parent(node)));
                rbtree_right_rotate(tr, rb_node_parent(rb_node_parent(node)));
            }
        } else {
            // node's parent is node's grand parent's right child; left uncle
            struct rb_node* uncle = rb_node_parent(rb_node_parent(node))->left;
            if (rb_node_is_red(uncle)) {
                // left uncle is red
                rb_node_set_black(rb_node_parent(node));
                rb_node_set_black(uncle);
                rb_node_set_red(rb_node_parent(rb_node_parent(node)));
                node = rb_node_parent(rb_node_parent(node));
            } else {
                // left uncle is black
                if (node == rb_node_parent(node)->left) {
                    // node is parent's left child
                    node = rb_node_parent(node);
                    rbtree_right_rotate(tr, node);
                }

                // node is parent's right child
                rb_node_is_black(rb_node_parent(node));
                rb_node_is_red(rb_node_parent(rb_node_parent(node)));
                rbtree_left_rotate(tr, rb_node_parent(rb_node_parent(node)));
            }
        }
    }
    rb_node_set_black(tr->root);
}

#include "stack.h"


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
    rbtree_set(tr, int_value(5));
    rbtree_set(tr, int_value(5));
    rbtree_set(tr, int_value(8));
    rbtree_set(tr, int_value(3));
    rbtree_set(tr, int_value(1));
    rbtree_set(tr, int_value(-6));
    rbtree_set(tr, int_value(7));
    rbtree_ldr(tr);
    close_rbtree(tr);
}