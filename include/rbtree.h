//
// Created by t5w0rd on 2019-04-16.
//

#ifndef ALGORITHM_RBTREE_H
#define ALGORITHM_RBTREE_H

#include "algorithm.h"


typedef struct rbtree RBTREE;

RBTREE* open_rbtree(COMPARE compare);
void close_rbtree(RBTREE* tr);
long rbtree_len(RBTREE* tr);
void rbtree_clear(RBTREE* tr);

VALUE rbtree_get(RBTREE* tr, VALUE key, int* ok);
void rbtree_set(RBTREE *tr, VALUE value);
VALUE rbtree_pop(RBTREE* tr, VALUE key, int* ok);

typedef struct rbnode RBNODE;
RBNODE* rbtree_open_node(RBTREE* tr, VALUE value, RBNODE* parent);
void rbtree_close_node(RBTREE* tr, RBNODE* node);
RBNODE** rbtree_fast_get(RBTREE *tr, VALUE key, RBNODE** parent);
void rbtree_fast_set(RBTREE *tr, RBNODE** where, RBNODE* node);
VALUE rbtree_fast_pop(RBTREE *tr, RBNODE *node);

#endif //ALGORITHM_RBTREE_H
