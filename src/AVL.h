#ifndef AVL_h  // 如果未定义 AVL_h 宏
#define AVL_h  // 定义 AVL_h 宏

// 定义一个名为 AVL 的结构体，用于表示 AVL 树的节点
typedef struct AVL {
    char *domain;  // 域名
    char *type;    // 类型
    char *ip;      // IP 地址
    int height;    // 节点高度
    struct AVL* next;  // 指向下一个节点的指针（用于处理重复域名）
    struct AVL *lchild, *rchild;  // 指向左子节点和右子节点的指针
} AVL;

AVL *root;  // 声明一个指向 AVL 树根节点的指针

// 函数声明
int height(AVL *node);  // 获取节点高度
int max(int a, int b);  // 返回两个整数中的较大值
AVL *LLRotation(AVL *node);  // 左左旋转
AVL *RRRotation(AVL *node);  // 右右旋转
AVL *LRRotation(AVL *node);  // 左右旋转
AVL *RLRotation(AVL *node);  // 右左旋转
AVL *insert(AVL *root, char *domain, char *type, char *ip);  // 插入节点
extern AVL* free_avl(AVL *root);  // 释放 AVL 树的内存
void Preorder(AVL *root);  // 前序遍历 AVL 树

#endif /* AVL_h */  // 结束条件编译块
