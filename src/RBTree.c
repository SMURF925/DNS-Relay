// RBTree.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "RBTree.h"

static RBTreeNode* createNode(char *key, char *value, NodeColor color) {
    RBTreeNode *node = (RBTreeNode*)malloc(sizeof(RBTreeNode));
    node->key = strdup(key);
    node->value = strdup(value);
    node->color = color;
    node->left = node->right = node->parent = NULL;
    return node;
}

static void destroyNode(RBTreeNode *node) {
    if (node) {
        free(node->key);
        free(node->value);
        free(node);
    }
}

RBTree* createRBTree() {
    RBTree *tree = (RBTree*)malloc(sizeof(RBTree));
    tree->root = NULL;
    return tree;
}

void destroyRBTree(RBTree *tree) {
    // Implement a function to destroy all nodes in the tree
    // This is left as an exercise for brevity
    free(tree);
}

// Function prototypes for internal helper functions
static void leftRotate(RBTree *tree, RBTreeNode *x);
static void rightRotate(RBTree *tree, RBTreeNode *y);
static void rbInsertFixup(RBTree *tree, RBTreeNode *z);

void rbInsert(RBTree *tree, char *key, char *value) {
    RBTreeNode *z = createNode(key, value, RED);
    RBTreeNode *y = NULL;
    RBTreeNode *x = tree->root;

    while (x != NULL) {
        y = x;
        if (strcmp(z->key, x->key) < 0) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    z->parent = y;
    if (y == NULL) {
        tree->root = z;
    } else if (strcmp(z->key, y->key) < 0) {
        y->left = z;
    } else {
        y->right = z;
    }

    rbInsertFixup(tree, z);
}

// Left Rotate and Right Rotate
static void leftRotate(RBTree *tree, RBTreeNode *x) {
    RBTreeNode *y = x->right;
    x->right = y->left;
    if (y->left != NULL) {
        y->left->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == NULL) {
        tree->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}

static void rightRotate(RBTree *tree, RBTreeNode *y) {
    RBTreeNode *x = y->left;
    y->left = x->right;
    if (x->right != NULL) {
        x->right->parent = y;
    }
    x->parent = y->parent;
    if (y->parent == NULL) {
        tree->root = x;
    } else if (y == y->parent->right) {
        y->parent->right = x;
    } else {
        y->parent->left = x;
    }
    x->right = y;
    y->parent = x;
}

// Insertion fixup
static void rbInsertFixup(RBTree *tree, RBTreeNode *z) {
    while (z->parent != NULL && z->parent->color == RED) {
        if (z->parent == z->parent->parent->left) {
            RBTreeNode *y = z->parent->parent->right;
            if (y != NULL && y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    leftRotate(tree, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rightRotate(tree, z->parent->parent);
            }
        } else {
            RBTreeNode *y = z->parent->parent->left;
            if (y != NULL && y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rightRotate(tree, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                leftRotate(tree, z->parent->parent);
            }
        }
    }
    tree->root->color = BLACK;
}

// Search and Delete functions to be implemented similarly...
