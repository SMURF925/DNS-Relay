#include <stdio.h>     // 包含标准输入输出库
#include <stdlib.h>    // 包含标准库，用于内存分配和转换
#include <string.h>    // 包含字符串操作库
#include "dns.h"       // 包含自定义的DNS头文件
#include "AVL.h"       // 包含自定义的AVL树头文件

#define MAX_FILE_LINE_WIDTH 128  // 定义文件行的最大宽度

// 在数据库中搜索DNS记录的函数
enum TYPE search_database(char name[], char *resultIP[], enum TYPE requestType, int *resnum) {
    AVL *tmp = malloc(sizeof(AVL));  // 分配内存用于临时AVL节点
    AVL *t = malloc(sizeof(AVL));    // 分配内存用于遍历AVL节点
    tmp = root;  // 初始化临时节点为AVL树的根节点
    while(tmp) {  // 遍历AVL树
        if(strcmp(name, tmp->domain) == 0) {  // 如果找到匹配的域名
            t = tmp;  // 初始化遍历节点为当前节点
            *resnum = 0;  // 初始化结果数量为0
            while(t) {  // 遍历所有匹配的节点
                if(strcmp(t->ip, "0.0.0.0") == 0 || strcmp(t->ip, "0:0:0:0") == 0) {  // 如果IP地址为0.0.0.0或0:0:0:0，表示被阻止
                    printf("Database block\n");  // 打印阻止信息
                    return BLOCK;  // 返回BLOCK状态
                } else {
                    if(strcmp(t->type, "A") == 0 && requestType == A) {  // 如果记录类型为A且请求类型为A
                        strcpy(resultIP[(*resnum)++], t->ip);  // 将IP地址复制到结果数组，并增加结果数量
                    } else if(strcmp(t->type, "AAAA") == 0 && requestType == AAAA) {  // 如果记录类型为AAAA且请求类型为AAAA
                        strcpy(resultIP[(*resnum)++], t->ip);  // 将IP地址复制到结果数组，并增加结果数量
                    }
                    t = t->next;  // 移动到下一个匹配的节点
                    if(*resnum == 10) {  // 如果结果数量达到10
                        break;  // 退出循环
                    }
                }
            }
            return requestType;  // 返回请求类型
        } else if(strcmp(name, tmp->domain) > 0) {  // 如果当前节点的域名大于搜索的域名
            tmp = tmp->rchild;  // 移动到右子节点
        } else {
            tmp = tmp->lchild;  // 移动到左子节点
        }
    }
    return NOT_EXIST;  // 如果未找到匹配的域名，返回NOT_EXIST状态
}
