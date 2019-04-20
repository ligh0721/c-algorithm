//
// Created by t5w0rd on 2019-04-16.
//

#include <assert.h>
#include <math.h>
#include "rbtree.h"
#include "stack.h"


struct rbnode {
    VALUE value;
    union {
        unsigned long parent_and_color;  // 父节点的指针以及颜色
        struct {
            unsigned long color:1;
        };
    };
#define RB_RED      0
#define RB_BLACK    1
    struct rbnode* right;
    struct rbnode* left;
} __attribute__((aligned(sizeof(long))));


static inline RBNODE* open_rbnode(VALUE value) {
    struct rbnode* ret = NEW(struct rbnode);
    ret->value = value;
    ret->left = ret->right = NULL;
    return ret;
}

static inline void close_rbnode(RBNODE* node) {
    DELETE(node);
}

static inline struct rbnode* rbnode_parent(struct rbnode* node) {
    return (struct rbnode*)((node)->parent_and_color & ~1);
}

static inline int rbnode_is_red(struct rbnode* node) {
    return !(node->parent_and_color & 1);
}

static inline int rbnode_is_black(struct rbnode* node) {
    return node->parent_and_color & 1;
}

static inline void rbnode_set_red(struct rbnode* node) {
    node->parent_and_color &= ~1;
}

static inline void rbnode_set_black(struct rbnode* node) {
    node->parent_and_color |= 1;
}

// 只设置父节点地址
static inline void rbnode_set_parent(struct rbnode *node, struct rbnode *parent) {
    node->parent_and_color = (node->parent_and_color & 1) | (unsigned long)parent;
}

static inline void rbnode_copy_color(struct rbnode *node, struct rbnode *src) {
    node->parent_and_color = (node->parent_and_color & ~1) | (src->parent_and_color & 1);
}


struct rbtree {
    struct rbnode* root;
    struct rbnode* leaf;
    COMPARE compare;
    long length;
};

RBTREE* open_rbtree(COMPARE compare) {
    struct rbtree* ret = NEW(struct rbtree);
    assert(ret != NULL);
    ret->leaf = open_rbnode(NULL_VALUE);
    ret->leaf->parent_and_color = RB_BLACK;
    ret->root = ret->leaf;
    ret->compare = compare;
    ret->length = 0;
    return ret;
}

void close_rbtree(RBTREE* tr) {
    assert(tr != NULL);
    rbtree_clear(tr);
    close_rbnode(tr->leaf);
    DELETE(tr);
}

long rbtree_len(RBTREE* tr) {
    assert(tr != NULL);
    return tr->length;
}

static inline int rbtree_empty(RBTREE* tr) {
    return tr->root == tr->leaf;
}

void rbtree_clear(RBTREE* tr) {
    assert(tr != NULL);
    if (rbtree_empty(tr)) {
        return;
    }
    STACK* st = open_stack(log2(rbtree_len(tr)+1)*2);
    struct rbnode* top = tr->root;
    struct rbnode* right;
    stack_push(st, ptr_value(top));
    while (stack_len(st) > 0) {
        while (top->left != tr->leaf) {
            top = top->left;
            stack_push(st, ptr_value(top));
        }
        do {
            top = (struct rbnode*)stack_pop(st, NULL).ptr_value;
            right = top->right;
            close_rbnode(top);
        } while (right == tr->leaf && stack_len(st) > 0);
        if (right != tr->leaf) {
            top = right;
            stack_push(st, ptr_value(top));
        }
    }
    close_stack(st);
    tr->root = NULL;
    tr->length = 0;
}

// default red
inline RBNODE* rbtree_open_node(RBTREE* tr, VALUE value, RBNODE* parent) {
    struct rbnode* ret = NEW(struct rbnode);
    ret->value = value;
    ret->parent_and_color = (unsigned long)parent;
    ret->left = ret->right = tr->leaf;
    return ret;
}

inline void rbtree_close_node(RBTREE* tr, RBNODE* node) {
    DELETE(node);
}

static inline void rbtree_left_rotate(RBTREE* tr, struct rbnode* node) {
    assert(node->right != tr->leaf);
    struct rbnode* right = node->right;
    struct rbnode* parent = rbnode_parent(node);

    node->right = right->left;
    if (right->left != tr->leaf) {
        rbnode_set_parent(right->left, node);
    }

    rbnode_set_parent(right, parent);
    if (node == tr->root) {
        tr->root = right;
    } else if (node == parent->left) {
        parent->left = right;
    } else {
        parent->right = right;
    }
    right->left = node;
    rbnode_set_parent(node, right);
}

static inline void rbtree_right_rotate(RBTREE* tr, struct rbnode* node) {
    assert(node->left != tr->leaf);
    struct rbnode* left = node->left;
    struct rbnode* parent = rbnode_parent(node);

    node->left = left->right;
    if (left->right != tr->leaf) {
        rbnode_set_parent(left->right, node);
    }

    rbnode_set_parent(left, parent);
    if (node == tr->root) {
        tr->root = left;
    } else if (node == parent->right) {
        parent->right = left;
    } else {
        parent->left = left;
    }
    left->right = node;
    rbnode_set_parent(node, left);
}
/*
 *  if *ret == leaf, not found (ret == &root)
 *  else found
 */
inline RBNODE** rbtree_fast_get(RBTREE *tr, const VALUE key, RBNODE** parent) {
    struct rbnode** ret = &tr->root;
    struct rbnode* leaf = tr->leaf;
    if (*ret == leaf) {
        if (parent) {
            *parent = NULL;
        }
        return ret;
    }

    register COMPARE compare = tr->compare;
    for (;;) {
        struct rbnode* node = *ret;
        register int cmp = compare(key, node->value);
        if (cmp == 0) {
            if (parent) {
                *parent = rbnode_parent(node);
            }
            return ret;
        }
        ret = cmp < 0 ? &node->left : &node->right;
        if (*ret == leaf) {
            if (parent) {
                *parent = node;
            }
            return ret;
        }
    }
}

void rbtree_fast_set(RBTREE *tr, RBNODE** where, RBNODE* node) {
    assert(tr != NULL);
    assert(*where == tr->leaf);
    *where = node;
    tr->length++;
    if (where == &tr->root) {
        rbnode_set_black(tr->root);
        return;
    }

    // re-balance tree
    while (node != tr->root && rbnode_is_red(rbnode_parent(node))) {
        // node isnot root and node's parent is red
        if (rbnode_parent(node) == rbnode_parent(rbnode_parent(node))->left) {
            // node's parent is node's grand parent's left child; right uncle
            struct rbnode* uncle = rbnode_parent(rbnode_parent(node))->right;
            if (rbnode_is_red(uncle)) {
                // right uncle is red
                rbnode_set_black(rbnode_parent(node));
                rbnode_set_black(uncle);
                rbnode_set_red(rbnode_parent(rbnode_parent(node)));
                node = rbnode_parent(rbnode_parent(node));
            } else {
                // right uncle is black
                if (node == rbnode_parent(node)->right) {
                    // node is parent's right child
                    node = rbnode_parent(node);
                    rbtree_left_rotate(tr, node);
                }

                // node is parent's left child
                rbnode_set_black(rbnode_parent(node));
                rbnode_set_red(rbnode_parent(rbnode_parent(node)));
                rbtree_right_rotate(tr, rbnode_parent(rbnode_parent(node)));
            }
        } else {
            // node's parent is node's grand parent's right child; left uncle
            struct rbnode* uncle = rbnode_parent(rbnode_parent(node))->left;
            if (rbnode_is_red(uncle)) {
                // left uncle is red
                rbnode_set_black(rbnode_parent(node));
                rbnode_set_black(uncle);
                rbnode_set_red(rbnode_parent(rbnode_parent(node)));
                node = rbnode_parent(rbnode_parent(node));
            } else {
                // left uncle is black
                if (node == rbnode_parent(node)->left) {
                    // node is parent's left child
                    node = rbnode_parent(node);
                    rbtree_right_rotate(tr, node);
                }

                // node is parent's right child
                rbnode_set_black(rbnode_parent(node));
                rbnode_set_red(rbnode_parent(rbnode_parent(node)));
                rbtree_left_rotate(tr, rbnode_parent(rbnode_parent(node)));
            }
        }
    }
    rbnode_set_black(tr->root);
}

VALUE rbtree_fast_pop(RBTREE *tr, RBNODE *node) {
    assert(tr != NULL);
    assert(node != NULL);
    VALUE ret = node->value;
    struct rbnode* leaf = tr->leaf;
    struct rbnode* temp;
    struct rbnode* subst;

    // a Binary Search Tree delete
    if (node->left == leaf) {
        temp = node->right;
        subst = node;
    } else if (node->right == leaf) {
        temp = node->left;
        subst = node;
    } else {
        for (subst=node->right; subst->left!=leaf; subst=subst->left);
        if (subst->left != leaf) {
            temp = subst->left;
        } else {
            temp = subst->right;
        }
    }

    if (subst == tr->root) {
        tr->root = temp;
        rbnode_set_black(temp);
        tr->length--;
        return ret;
    }

    unsigned long red = rbnode_is_red(subst);

    if (subst == rbnode_parent(subst)->left) {
        rbnode_parent(subst)->left = temp;
    } else {
        rbnode_parent(subst)->right = temp;
    }

    if (subst == node) {
        rbnode_set_parent(temp, rbnode_parent(subst));
    } else {
        if (rbnode_parent(subst) == node) {
            rbnode_set_parent(temp, subst);
        } else {
            rbnode_set_parent(temp, rbnode_parent(subst));
        }

        subst->left = node->left;
        subst->right = node->right;
        subst->parent_and_color = node->parent_and_color;

        if (node == tr->root) {
            tr->root = subst;
        } else {
            if (node == rbnode_parent(node)->left) {
                rbnode_parent(node)->left = subst;
            } else {
                rbnode_parent(node)->right = subst;
            }
        }

        if (subst->left != leaf) {
            rbnode_set_parent(subst->left, subst);
        }

        if (subst->right != leaf) {
            rbnode_set_parent(subst->right, subst);
        }
    }

    tr->length--;

    if (red) {
        return ret;
    }

    // re-balance tree
    struct rbnode* w;
    while (temp != tr->root && rbnode_is_black(temp)) {
        if (temp == rbnode_parent(temp)->left) {
            w = rbnode_parent(temp)->right;

            if (rbnode_is_red(w)) {
                rbnode_set_black(w);
                rbnode_set_red(rbnode_parent(temp));
                rbtree_left_rotate(tr, rbnode_parent(temp));
                w = rbnode_parent(temp)->right;
            }

            if (rbnode_is_black(w->left) && rbnode_is_black(w->right)) {
                rbnode_set_red(w);
                temp = rbnode_parent(temp);
            } else {
                if (rbnode_is_black(w->right)) {
                    rbnode_set_black(w->left);
                    rbnode_set_red(w);
                    rbtree_right_rotate(tr, w);
                    w = rbnode_parent(temp)->right;
                }

                rbnode_copy_color(w, rbnode_parent(temp));
                rbnode_set_black(rbnode_parent(temp));
                rbnode_set_black(w->right);
                rbtree_left_rotate(tr, rbnode_parent(temp));
                temp = tr->root;
            }

        } else {
            w = rbnode_parent(temp)->left;

            if (rbnode_is_red(w)) {
                rbnode_set_black(w);
                rbnode_set_red(rbnode_parent(temp));
                rbtree_right_rotate(tr, rbnode_parent(temp));
                w = rbnode_parent(temp)->left;
            }

            if (rbnode_is_black(w->left) && rbnode_is_black(w->right)) {
                rbnode_set_red(w);
                temp = rbnode_parent(temp);
            } else {
                if (rbnode_is_black(w->left)) {
                    rbnode_set_black(w->right);
                    rbnode_set_red(w);
                    rbtree_left_rotate(tr, w);
                    w = rbnode_parent(temp)->left;
                }

                rbnode_copy_color(w, rbnode_parent(temp));
                rbnode_set_black(rbnode_parent(temp));
                rbnode_set_black(w->left);
                rbtree_right_rotate(tr, rbnode_parent(temp));
                temp = tr->root;
            }
        }
    }

    rbnode_set_black(temp);
    return ret;
}

VALUE rbtree_get(RBTREE* tr, const VALUE key, int* ok) {
    RBNODE* where = *rbtree_fast_get(tr, key, NULL);
    if (where == tr->leaf) {
        if (ok) {
            *ok = 0;
        }
        return NULL_VALUE;
    }
    if (ok) {
        *ok = 1;
    }
    return where->value;
}

void rbtree_set(RBTREE *tr, const VALUE value) {
    assert(tr != NULL);
    struct rbnode* leaf = tr->leaf;
    struct rbnode* node;
    RBNODE* parent;
    RBNODE** where = rbtree_fast_get(tr, value, &parent);
    if (*where != leaf) {
        (*where)->value = value;
        return;
    }
    node = rbtree_open_node(tr, value, parent);
    rbtree_fast_set(tr, where, node);
}

VALUE rbtree_pop(RBTREE* tr, const VALUE key, int* ok) {
    assert(tr != NULL);
    RBNODE* search = *rbtree_fast_get(tr, key, NULL);
    if (search == NULL || search == tr->leaf) {
        if (ok) {
            *ok = 0;
        }
        return NULL_VALUE;
    }
    if (ok) {
        *ok = 1;
    }
    VALUE ret = rbtree_fast_pop(tr, search);
    close_rbnode(search);
    return ret;
}

void rbtree_ldr(RBTREE* tr, RBTREE_TRAVERSE traverse, void* param) {
    assert(tr != NULL);
    if (rbtree_empty(tr)) {
        return;
    }
    STACK* st = open_stack(log2(rbtree_len(tr)+1)*2);
    struct rbnode* top = tr->root;
    stack_push(st, ptr_value(top));
    while (stack_len(st) > 0) {
        while (top->left != tr->leaf) {
            top = top->left;
            stack_push(st, ptr_value(top));
        }
        do {
            top = (struct rbnode*)stack_pop(st, NULL).ptr_value;
            if (traverse(top->value, param)) {
                close_stack(st);
                return;
            }
        } while (top->right == tr->leaf && stack_len(st) > 0);
        if (top->right != tr->leaf) {
            top = top->right;
            stack_push(st, ptr_value(top));
        }
    }
    close_stack(st);
}
