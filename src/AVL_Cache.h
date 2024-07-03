#ifndef AVL_Cache_h  // 如果未定义 AVL_Cache_h 宏
#define AVL_Cache_h  // 定义 AVL_Cache_h 宏

#include "dns.h"  // 包含 dns.h 头文件
#include "time.h"  // 包含 time.h 头文件

// 定义一个名为 CacheResponse 的结构体，用于表示缓存响应节点
typedef struct CacheResponse {
    Response *response;  // 指向响应数据的指针
    time_t timestamp;  // 时间戳，记录响应的时间
    int height;  // 节点高度
    struct CacheResponse *lchild, *rchild;  // 指向左子节点和右子节点的指针
} CacheResponse;

// 函数声明
CacheResponse *getRoot(Response *response);  // 创建新的根节点
CacheResponse* insertCache(CacheResponse *root, Response *data);  // 插入节点到缓存
CacheResponse* searchCache(CacheResponse *root, char* qName);  // 在缓存中搜索节点
CacheResponse* deleteCache(CacheResponse *root, CacheResponse *node);  // 删除缓存中的节点

CacheResponse *cacheRoot;  // 声明一个指向缓存根节点的指针

#endif /* AVL_Cache_h */  // 结束条件编译块
