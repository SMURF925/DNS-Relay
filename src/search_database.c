#include <stdio.h>     // 包含标准输入输出库
#include <stdlib.h>    // 包含标准库，用于内存分配和转换
#include <string.h>    // 包含字符串操作库
#include "dns.h"       // 包含自定义的DNS头文件
#include "RBTree.h"    // 包含自定义的红黑树头文件

#define MAX_FILE_LINE_WIDTH 128  // 定义文件行的最大宽度

extern RBTree *root;  // 定义红黑树根节点

// 在数据库中搜索DNS记录的函数
enum TYPE search_database(char name[], char *resultIP[], enum TYPE requestType, int *resnum) {
    RBTreeNode *tmp = root;  // 初始化临时节点为红黑树的根节点
    RBTreeNode *t = NULL;    // 定义遍历红黑树节点
    while(tmp) {  // 遍历红黑树
        if(strcmp(name, tmp->key) == 0) {  // 如果找到匹配的域名
            t = tmp;  // 初始化遍历节点为当前节点
            *resnum = 0;  // 初始化结果数量为0
            while(t) {  // 遍历所有匹配的节点
                if(strcmp(t->value, "0.0.0.0") == 0 || strcmp(t->value, "0:0:0:0") == 0) {  // 如果IP地址为0.0.0.0或0:0:0:0，表示被阻止
                    printf("Database block\n");  // 打印阻止信息
                    return BLOCK;  // 返回BLOCK状态
                } else {
                    if(requestType == A) {  // 如果请求类型为A
                        strcpy(resultIP[(*resnum)++], t->value);  // 将IP地址复制到结果数组，并增加结果数量
                    } else if(requestType == AAAA) {  // 如果请求类型为AAAA
                        strcpy(resultIP[(*resnum)++], t->value);  // 将IP地址复制到结果数组，并增加结果数量
                    }
                    // 红黑树节点没有 next 指针，因此直接跳出循环
                    if(*resnum == 10) {  // 如果结果数量达到10
                        break;  // 退出循环
                    }
                }
                break;  // 红黑树节点没有 next 指针，因此直接跳出循环
            }
            return requestType;  // 返回请求类型
        } else if(strcmp(name, tmp->key) > 0) {  // 如果当前节点的域名大于搜索的域名
            tmp = tmp->right;  // 移动到右子节点
        } else {
            tmp = tmp->left;  // 移动到左子节点
        }
    }
    return NOT_EXIST;  // 如果未找到匹配的域名，返回NOT_EXIST状态
}
