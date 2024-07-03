// AVL_Cache.h -> RBTree_Cache.h
#ifndef RBTREE_CACHE_H
#define RBTREE_CACHE_H

#include "RBTree.h"

typedef struct {
    RBTree *tree;
} RBTreeCache;

RBTreeCache* createRBTreeCache();
void destroyRBTreeCache(RBTreeCache *cache);

void rbCacheInsert(RBTreeCache *cache, char *key, char *value);
void rbCacheDelete(RBTreeCache *cache, char *key);
char* rbCacheSearch(RBTreeCache *cache, char *key);

#endif // RBTREE_CACHE_H
