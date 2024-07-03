// AVL_Cache.c -> RBTree_Cache.c
#include "RBTree_Cache.h"

RBTreeCache* createRBTreeCache() {
    RBTreeCache *cache = (RBTreeCache*)malloc(sizeof(RBTreeCache));
    cache->tree = createRBTree();
    return cache;
}

void destroyRBTreeCache(RBTreeCache *cache) {
    destroyRBTree(cache->tree);
    free(cache);
}

void rbCacheInsert(RBTreeCache *cache, char *key, char *value) {
    rbInsert(cache->tree, key, value);
}

void rbCacheDelete(RBTreeCache *cache, char *key) {
    rbDelete(cache->tree, key);
}

char* rbCacheSearch(RBTreeCache *cache, char *key) {
    return rbSearch(cache->tree, key);
}
