#include <stdio.h>   // 包含标准输入输出库
#include <stdlib.h>  // 包含标准库
#include <string.h>  // 包含字符串操作库
#include "AVL_Cache.h"  // 包含自定义的 AVL_Cache 头文件
#include <time.h>  // 包含时间操作库

// 获取节点高度
int getHeight(CacheResponse *node) {
    return node == NULL ? -1 : node->height;  // 如果节点为空返回-1，否则返回节点高度
}

// 获取两个整数的最大值
int getMax(int a, int b) {
    return a >= b ? a : b;  // 返回较大值
}

// 创建新的根节点
CacheResponse *getRoot(Response *response) {
    CacheResponse *root = malloc(sizeof(CacheResponse));  // 分配内存
    root->response = response;  // 设置响应
    root->lchild = root->rchild = NULL;  // 初始化左右子节点为空
    time_t now;
    root->timestamp = time(&now);  // 设置当前时间戳
    root->height = getMax(getHeight(root->lchild),getHeight(root->rchild)) + 1;  // 计算高度
    return root;  // 返回根节点
}

// 左左旋转
CacheResponse *LL_Rotation(CacheResponse *node) {
    CacheResponse *tmp = node->lchild;  // 临时指针指向左子节点
    node->lchild = tmp->rchild;  // 将tmp的右子节点设为node的左子节点
    tmp->rchild = node;  // 将node设为tmp的右子节点
    node->height = getMax(getHeight(node->lchild), getHeight(node->rchild)) + 1;  // 更新node高度
    tmp->height = getMax(getHeight(tmp->lchild), getHeight(tmp->rchild)) + 1;  // 更新tmp高度
    return tmp;  // 返回新的根节点
}

// 右右旋转
CacheResponse *RR_Rotation(CacheResponse *node) {
    CacheResponse *tmp = node->rchild;  // 临时指针指向右子节点
    node->rchild = tmp->lchild;  // 将tmp的左子节点设为node的右子节点
    tmp->lchild = node;  // 将node设为tmp的左子节点
    node->height = getMax(getHeight(node->lchild), getHeight(node->rchild)) + 1;  // 更新node高度
    tmp->height = getMax(getHeight(tmp->lchild), getHeight(tmp->rchild)) + 1;  // 更新tmp高度
    return tmp;  // 返回新的根节点
}

// 左右旋转
CacheResponse *LR_Rotation(CacheResponse *node) {
    node->lchild = RR_Rotation(node->lchild);  // 对左子节点进行右右旋转
    return LL_Rotation(node);  // 对节点进行左左旋转
}

// 右左旋转
CacheResponse *RL_Rotation(CacheResponse *node) {
    node->rchild = LL_Rotation(node->rchild);  // 对右子节点进行左左旋转
    return RR_Rotation(node);  // 对节点进行右右旋转
}

// 获取最大值节点
CacheResponse *maxMum(CacheResponse *node) {
    if(node == NULL) {
        return NULL;  // 如果节点为空，返回NULL
    }
    while(node->rchild != NULL) {
        node = node->rchild;  // 一直遍历到最右节点
    }
    return node;  // 返回最大值节点
}

// 获取最小值节点
CacheResponse *miniMum(CacheResponse *node) {
    if(node == NULL) {
        return NULL;  // 如果节点为空，返回NULL
    }
    while(node->lchild != NULL) {
        node = node->lchild;  // 一直遍历到最左节点
    }
    return node;  // 返回最小值节点
}

// 插入节点到缓存
CacheResponse* insertCache(CacheResponse *root, Response *data) {
    if(root == NULL) {
        root = (CacheResponse *)malloc(sizeof(CacheResponse));  // 分配内存
        if(root) {
            root->response = data;  // 设置响应
            root->lchild = root->rchild = NULL;  // 初始化左右子节点为空
            time_t now;
            root->timestamp = time(&now);  // 设置当前时间戳
            root->height = getMax(getHeight(root->lchild),getHeight(root->rchild)) + 1;  // 计算高度
            return root;
        }
        else
            return NULL;
    }
    else if(strcmp(data->question->qName, root->response->question->qName) < 0) {  // 插入到左子树
        root->lchild = insertCache(root->lchild, data);
        if(getHeight(root->lchild) - getHeight(root->rchild) == 2) {
            if(strcmp(data->question->qName, root->lchild->response->question->qName) < 0)
                root = LL_Rotation(root);  // 左左旋转
            else
                root = LR_Rotation(root);  // 左右旋转
        }
    }
    else if(strcmp(data->question->qName, root->response->question->qName) > 0) {  // 插入到右子树
        root->rchild = insertCache(root->rchild, data);
        if(getHeight(root->rchild) - getHeight(root->lchild) == 2) {
            if(strcmp(data->question->qName, root->rchild->response->question->qName) > 0)
                root = RR_Rotation(root);  // 右右旋转
            else
                root = RL_Rotation(root);  // 右左旋转
        }
    }
    root->height = getMax(getHeight(root->lchild),getHeight(root->rchild)) + 1;  // 更新高度
    return root;
}

// 在缓存中搜索节点
CacheResponse* searchCache(CacheResponse *root, char* qName) {
    CacheResponse *tmp = root;
    while(tmp) {
        if(strcmp(tmp->response->question->qName, qName) == 0) {
            return tmp;  // 找到节点返回
        }
        else if(strcmp(tmp->response->question->qName, qName) < 0) {
            tmp = tmp->rchild;  // 继续在右子树搜索
        }
        else {
            tmp = tmp->lchild;  // 继续在左子树搜索
        }
    }
    return NULL;  // 没找到返回NULL
}

// 删除缓存中的节点
CacheResponse* deleteCache(CacheResponse *root, CacheResponse *node) {
    if(node == NULL) {
        exit(0);  // 如果节点为空，退出程序
    }
    if(strcmp(node->response->question->qName, root->response->question->qName) > 0) {
        root->rchild = deleteCache(root->rchild, node);  // 在右子树中删除节点
        if(getHeight(root->lchild) - getHeight(root->rchild) == 2) {
            CacheResponse *tmp = root->lchild;
            if(getHeight(tmp->lchild) >= getHeight(tmp->rchild)) {
                root = LL_Rotation(root);  // 左左旋转
            }
            else {
                root = LR_Rotation(root);  // 左右旋转
            }
        }
    }
    else if(strcmp(node->response->question->qName, root->response->question->qName) < 0) {
        root->lchild = deleteCache(root->lchild, node);  // 在左子树中删除节点
        if(getHeight(root->rchild) - getHeight(root->lchild) == 2) {
            CacheResponse *tmp = root->rchild;
            if(getHeight(tmp->rchild) >= getHeight(tmp->lchild)) {
                root = RR_Rotation(root);  // 右右旋转
            }
            else {
                root = RL_Rotation(root);  // 右左旋转
            }
        }
    }
    else if(strcmp(node->response->question->qName, root->response->question->qName) == 0) {
        if(root->lchild != NULL && root->rchild != NULL) {  // 如果左右子节点都不为空
            if(getHeight(root->lchild) > getHeight(root->rchild)) {
                CacheResponse *max = maxMum(root->lchild);  // 获取左子树最大值节点
                root->response = max->response;  // 替换为最大值节点的数据
                root->timestamp = max->timestamp;
                root->lchild = deleteCache(root->lchild, node);  // 删除左子树中的最大值节点

            }
            else {
                CacheResponse *min = miniMum(root->rchild);  // 获取右子树最小值节点
                root->response = min->response;  // 替换为最小值节点的数据
                root->timestamp = min->timestamp;
                                root->rchild = deleteCache(root->rchild, node);  // 删除右子树中的最小值节点
            }
        }
        else {  // 如果只有一个子节点或没有子节点
            CacheResponse *p = root;
            root = root->lchild ? root->lchild : root->rchild;  // 替换为非空子节点
            if(p != NULL) {
                p->lchild = NULL;
                p->rchild = NULL;
                free(p);  // 释放原节点内存
            }
        }
    }
    return root;  // 返回根节点
}

