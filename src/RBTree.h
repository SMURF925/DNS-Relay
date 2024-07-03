// RBTree.h
#ifndef RBTREE_H
#define RBTREE_H

typedef enum { RED, BLACK } NodeColor;

typedef struct RBTreeNode {
    char *key;
    char *value;
    NodeColor color;
    struct RBTreeNode *left, *right, *parent;
} RBTreeNode;

typedef struct {
    RBTreeNode *root;
} RBTree;

RBTree* createRBTree();
void destroyRBTree(RBTree *tree);

void rbInsert(RBTree *tree, char *key, char *value);
void rbDelete(RBTree *tree, char *key);
char* rbSearch(RBTree *tree, char *key);

#endif // RBTREE_H
