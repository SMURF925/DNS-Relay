#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dns.h"
#include "AVL.h"

int same = 0; // 用于标记是否有重复的域名

// 获取节点高度
int height(AVL *node) {
    return node == NULL ? -1 : node->height;
}

// 获取两个整数的最大值
int max(int a, int b) {
    return a >= b ? a : b;
}

// 左左旋转
AVL *LLRotation(AVL *node) {
    AVL *tmp = node->lchild; // tmp指向node的左子节点
    node->lchild = tmp->rchild; // 将tmp的右子节点设置为node的左子节点
    tmp->rchild = node; // 将node设置为tmp的右子节点

    node->height = max(height(node->lchild), height(node->rchild)) + 1; // 更新node的高度
    tmp->height = max(height(tmp->lchild), height(tmp->rchild)) + 1; // 更新tmp的高度

    return tmp; // 返回新的根节点
}

// 右右旋转
AVL *RRRotation(AVL *node) {
    AVL *tmp = node->rchild; // tmp指向node的右子节点
    node->rchild = tmp->lchild; // 将tmp的左子节点设置为node的右子节点
    tmp->lchild = node; // 将node设置为tmp的左子节点

    node->height = max(height(node->lchild), height(node->rchild)) + 1; // 更新node的高度
    tmp->height = max(height(tmp->lchild), height(tmp->rchild)) + 1; // 更新tmp的高度

    return tmp; // 返回新的根节点
}

// 左右旋转
AVL *LRRotation(AVL *node) {
    node->lchild = RRRotation(node->lchild); // 对node的左子节点进行右旋转
    return LLRotation(node); // 对node进行左旋转
}

// 右左旋转
AVL *RLRotation(AVL *node) {
    node->rchild = LLRotation(node->rchild); // 对node的右子节点进行左旋转
    return RRRotation(node); // 对node进行右旋转
}

// 插入节点
AVL *insert(AVL *root, char *domain, char *type, char *ip) {
//    printf("%s\n", ip);
    if(root == NULL) {
        root = (AVL *)malloc(sizeof(AVL)); // 分配新的节点
        if(root) {
            root->domain = malloc(sizeof(char) * sizeof(domain));
            root->type = malloc(sizeof(char) * sizeof(type));
            root->ip = malloc(sizeof(char) * sizeof(ip));
            strcpy(root->domain, domain); // 复制域名
            strcpy(root->type, type); // 复制类型
            strcpy(root->ip, ip); // 复制IP地址
            root->lchild = root->next = root->rchild = NULL; // 初始化子节点和next指针
            root->height = max(height(root->lchild),height(root->rchild)) + 1; // 计算高度
            return root;
        }
        else
            return NULL;
    }
    else if(strcmp(domain, root->domain) < 0) { // 插入左子树
        root->lchild = insert(root->lchild, domain, type, ip);
        if(height(root->lchild) - height(root->rchild) == 2) {
            if(strcmp(domain, root->lchild->domain) < 0)
                root = LLRotation(root); // 左左旋转
            else
                root = LRRotation(root); // 左右旋转
        }
    }
    else if(strcmp(domain, root->domain) > 0) { // 插入右子树
        root->rchild = insert(root->rchild, domain, type, ip);
        if(height(root->rchild) - height(root->lchild) == 2) {
            if(strcmp(domain, root->rchild->domain) > 0)
                root = RRRotation(root); // 右右旋转
            else
                root = RLRotation(root); // 右左旋转
        }
    }
    else { // 处理重复的域名
        same = 1;
        AVL *tmpNode = malloc(sizeof(AVL));
        tmpNode->domain = malloc(sizeof(char) * sizeof(domain));
        tmpNode->type = malloc(sizeof(char) * sizeof(type));
        tmpNode->ip = malloc(sizeof(char) * sizeof(ip));
        strcpy(tmpNode->domain, domain); // 复制域名
        strcpy(tmpNode->type, type); // 复制类型
        strcpy(tmpNode->ip, ip); // 复制IP地址
        tmpNode->lchild = tmpNode->rchild = NULL; // 初始化子节点
        tmpNode->height = 0; // 高度为0
        tmpNode->next = root->next; // 插入到next链表中
        root->next = tmpNode;
    }
    if(same == 0) {
        root->height = max(height(root->lchild),height(root->rchild)) + 1; // 更新高度
    }
    return root;
}

// 释放AVL树内存
AVL* free_avl(AVL *root) {
    if(root) {
        free_avl(root->lchild); // 递归释放左子树
        free_avl(root->rchild); // 递归释放右子树
        free(root); // 释放当前节点
    }
    return NULL;
}

// 前序遍历AVL树
void Preorder(AVL *root) {
    if(!root)
        return ;
    AVL *tmp = root;
    while(tmp) {
        printf("%s %s %s\n", tmp->domain, tmp->type, tmp->ip); // 打印节点信息
        tmp = tmp->next;
    }
    free(tmp);
    Preorder(root->lchild); // 递归遍历左子树
    Preorder(root->rchild); // 递归遍历右子树
}
