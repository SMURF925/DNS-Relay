#include <stdio.h>    // 包含标准输入输出库
#include <sys/types.h>  // 包含系统数据类型定义
#include <sys/socket.h>  // 包含套接字函数定义
#include <arpa/inet.h>  // 包含网络地址转换函数
#include <unistd.h>  // 包含Unix标准函数定义
#include "dns.h"  // 包含自定义的DNS相关头文件
#include <stdlib.h>  // 包含标准库，用于内存分配和转换
#include <string.h>  // 包含字符串操作库
#include <ctype.h>  // 包含字符操作函数
#include <pthread.h>  // 包含线程操作函数
#include "AVL.h"  // 包含自定义的AVL树相关头文件
#include "debugging_info.h"  // 包含调试信息相关头文件
#include "AVL_Cache.h"  // 包含自定义的AVL缓存相关头文件

#define MAX_FILE_LINE_WIDTH 128  // 定义文件行最大宽度
#define SERVICE_THREAD_NUM 5  // 定义服务线程数量

pthread_mutex_t mutex_lock;  // 定义互斥锁
char *serverIP;  // 定义服务器IP地址指针

int main(int argc, const char * argv[]) {
    pthread_mutex_init(&mutex_lock, NULL);  // 初始化互斥锁
    printf("Starting DNS relay......\n");  // 输出启动信息
    printf("Mutex lock initialized.\n");  // 输出互斥锁初始化信息
    cacheRoot = NULL;  // 初始化缓存根节点
    serverIP = malloc(sizeof(char) * 100);  // 分配服务器IP地址内存
    if(strcmp(argv[1], "-d") == 0)  // 如果命令行参数为"-d"
    {
        set_up(1);  // 设置调试级别为1
        strcpy(serverIP, argv[2]);  // 复制服务器IP地址
    }
    else if(strcmp(argv[1], "-dd") == 0)  // 如果命令行参数为"-dd"
    {
        set_up(2);  // 设置调试级别为2
        strcpy(serverIP, argv[2]);  // 复制服务器IP地址
    }
    else
    {
        set_up(0);  // 设置调试级别为0
        strcpy(serverIP, argv[1]);  // 复制服务器IP地址
    }
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);  // 创建UDP套接字
    struct sockaddr_in addr;  // 定义套接字地址结构
    addr.sin_family = AF_INET;  // 设置地址族为IPv4
    addr.sin_port = htons(53);  // 设置端口号为53
    addr.sin_addr.s_addr = htonl(INADDR_ANY);  // 设置IP地址为本地任意地址

    char file_name[] = "./database.txt";  // 定义数据库文件名
    FILE *fin;  // 定义文件指针
    char *one_line;  // 定义文件行指针
    char *domain = NULL;  // 初始化域名指针
    one_line = (char*)malloc(MAX_FILE_LINE_WIDTH * sizeof(char));  // 分配行内存
    char *IP = NULL;  // 初始化IP地址指针
    int f = 0;  // 初始化标志
    root = (AVL*)malloc(sizeof(AVL));  // 分配AVL树节点内存
    if((fin = fopen(file_name, "r")) == NULL)  // 打开数据库文件
    {
        printf("can not open file %s !\n", file_name);  // 输出错误信息
    }
    char *type = NULL;  // 初始化类型指针

    while(fgets(one_line, MAX_FILE_LINE_WIDTH, fin) != NULL)  // 逐行读取文件
    {
        int first = 1;  // 初始化标志
        int len = strlen(one_line);  // 获取行长度
        if(one_line[len - 1] == '\n')  // 如果行末尾是换行符
        {
            one_line[len - 1] = 0;  // 替换为字符串结束符
        }

        for(int i = 0; i < len; i++)  // 遍历行中的每个字符
        {
            if(one_line[i] == ' ')  // 如果遇到空格
            {
                one_line[i] = 0;  // 替换为空字符，分割域名和其他部分
                domain = one_line;  // 设置域名指针

                if(first)  // 如果是第一个空格
                {
                    type = &one_line[i + 1];  // 设置类型指针
                    first = 0;  // 重置标志
                }
                else
                {
                    IP = &one_line[i + 1];  // 设置IP地址指针
                    one_line[len] = 0;  // 末尾加上空字符
                }
            }
        }

        if(f == 0) {  // 如果是第一个节点
            f = 1;  // 设置标志
            root->domain = malloc(sizeof(char) * sizeof(domain));  // 分配域名内存
            root->type = malloc(sizeof(char) * sizeof(type));  // 分配类型内存
            root->ip = malloc(sizeof(char) * sizeof(IP));  // 分配IP地址内存
            strcpy(root->domain, domain);  // 复制域名
            strcpy(root->type, type);  // 复制类型
            strcpy(root->ip, IP);  // 复制IP地址
            root->next = NULL;  // 初始化下一个节点指针
            root->lchild = NULL;  // 初始化左子节点指针
            root->rchild = NULL;  // 初始化右子节点指针
            root->height = 0;  // 初始化高度
        }
        else {
            root = insert(root, domain, type, IP);  // 插入节点到AVL树
        }

    }

    int ret = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));  // 绑定套接字地址
    if(ret < 0) {
        printf("bind error\n");  // 输出绑定错误信息
        return -1;  // 返回错误代码
    }
    printf("AVL tree build successfully.\n");  // 输出AVL树构建成功信息
    int num_of_threads = SERVICE_THREAD_NUM;  // 定义服务线程数量
    printf("Create %d service threads.\n", num_of_threads);  // 输出创建线程信息

    fflush(stdout);  // 刷新标准输出缓冲区
    free(one_line);  // 释放行内存
    pthread_t tid;  // 定义线程ID

    for(int i = 0; i < SERVICE_THREAD_NUM; i++)  // 创建服务线程
    {
        int arg = sockfd;  // 设置线程参数
        pthread_create(&tid, NULL, service_thread, &arg);  // 创建线程
    }
    pthread_join(tid, NULL);  // 等待线程结束

}
