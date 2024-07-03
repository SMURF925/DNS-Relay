#include <stdio.h>    // 包含标准输入输出库
#include "dns.h"      // 包含自定义的DNS头文件
#include <string.h>   // 包含字符串操作库
#include <ctype.h>    // 包含字符操作库
#include <stdlib.h>   // 包含标准库，用于内存分配和转换
#include <sys/types.h>  // 包含系统数据类型定义
#include <sys/socket.h>  // 包含套接字函数定义
#include <arpa/inet.h>  // 包含网络地址转换函数
#include <unistd.h>   // 包含Unix标准函数定义
#include <time.h>     // 包含时间库
#include "debugging_info.h"  // 包含调试信息头文件
#include "packet_parser.h"   // 包含包解析头文件
#include "AVL_Cache.h"   // 包含自定义的AVL缓存头文件
#include <pthread.h>     // 包含线程操作函数
#include <sys/wait.h>    // 包含进程等待函数

extern pthread_mutex_t mutex_lock;  // 声明外部互斥锁
extern char *serverIP;  // 声明外部服务器IP地址指针
int get_local_success = 0;  // 初始化本地DNS通信成功标志为0
void *communicate_with_local_DNS(void * buffer);  // 声明与本地DNS通信的函数

// 正常处理DNS请求函数
enum TYPE normal_handling(Response **response, DNSHeader *header, DNSQuestion *question, char buf[], int *response_length, int recv_num) {
    char* query_name = malloc(sizeof(char) * 100);  // 分配内存用于保存查询名称
    
    convert_string(question->qName, query_name);  // 转换查询名称
    
    char **resultIP = malloc(sizeof(char *) * 10);  // 分配内存用于保存查询结果IP
    for(int i = 0 ; i < 10; i++) {
        resultIP[i] = malloc(sizeof(char) * 50);  // 为每个IP分配内存
    }
    
    char type[50];  // 定义类型数组
    int resnum;  // 定义结果数量
    enum TYPE status = search_database(query_name, resultIP, question->qType, &resnum);  // 搜索数据库获取查询结果
    free(query_name);  // 释放查询名称内存

    if(status == A || status == AAAA) {  // 如果查询类型为A或AAAA
        (*response)->header = header;  // 设置响应头部
        (*response)->header->flags = 0x8081;  // 设置响应标志位
        short anCnt = resnum;  // 设置回答计数
        convertEndian((char *)&anCnt, 2);  // 转换字节序
        (*response)->header->anCnt = anCnt;  // 设置回答计数
        (*response)->header->qdCnt = 0x0100;  // 设置问题计数
        (*response)->header->nsCnt = 0x0000;  // 设置权威记录计数
        (*response)->header->arCnt = 0x0000;  // 设置附加记录计数
        (*response)->question = question;  // 设置问题
        DNSRR **pRR = &((*response)->firstRR);  // 设置指向第一个资源记录的指针
        for(int i = 0 ; i < resnum; i++) {
            *pRR = malloc(sizeof(DNSRR));  // 为资源记录分配内存
            (*pRR)->rName = question->qName;  // 设置资源记录名称
            (*pRR)->rType = question->qType;  // 设置资源记录类型
            (*pRR)->rClass = question->qClass;  // 设置资源记录类
            (*pRR)->ttl = 0x00040000;  // 设置生存时间
            if((*pRR)->rType == A)
                (*pRR)->rdLen = 0x0400;  // 设置资源数据长度为IPv4地址
            else
                (*pRR)->rdLen = 0x1000;  // 设置资源数据长度为IPv6地址
            (*pRR)->rData = resultIP[i];  // 设置资源数据
            pRR = &((*pRR)->next);  // 指向下一个资源记录
        }
        *pRR = NULL;  // 设置资源记录末尾为NULL
        
        return A;  // 返回A类型
    }
    else if(status == BLOCK) {  // 如果状态为BLOCK
        (*response)->header = header;  // 设置响应头部
        (*response)->header->flags = 0x8381;  // 设置响应标志位
        (*response)->header->anCnt = 0x0000;  // 设置回答计数为0
        (*response)->header->qdCnt = 0x0100;  // 设置问题计数
        (*response)->header->nsCnt = 0x0000;  // 设置权威记录计数为0
        (*response)->header->arCnt = 0x0000;  // 设置附加记录计数为0
        (*response)->question = question;  // 设置问题
        (*response)->firstRR = NULL;  // 设置资源记录为NULL
        return BLOCK;  // 返回BLOCK
    }
    else if(status == NOT_EXIST) {  // 如果状态为NOT_EXIST
        pthread_mutex_lock(&mutex_lock);  // 加锁
        CacheResponse *response_cache;
        if(cacheRoot == NULL)
            response_cache = NULL;  // 如果缓存根节点为空，设置响应缓存为NULL
        else
            response_cache = searchCache(cacheRoot, question->qName);  // 否则搜索缓存

        int need_local_DNS = 0;  // 初始化需要本地DNS标志为0
        if(response_cache != NULL) {  // 如果找到缓存
            time_t curtime;
            time(&curtime);  // 获取当前时间
            time_t pass = curtime - response_cache->timestamp;  // 计算时间差
            DNSRR * rr = response_cache->response->firstRR;
            while(rr) {  // 遍历资源记录
                int ttl = rr->ttl;
                convertEndian((char *)(&ttl), 4);  // 转换字节序
                ttl -= pass;  // 减去时间差
                if(ttl < 0) {  // 如果生存时间小于0
                    need_local_DNS = 1;  // 设置需要本地DNS标志为1
                    cacheRoot = deleteCache(cacheRoot, response_cache);  // 删除缓存
                    break;
                }
                response_cache->timestamp = curtime;  // 更新时间戳
                convertEndian((char *)(&ttl), 4);  // 转换字节序
                rr->ttl = ttl;  // 更新生存时间
                rr = rr->next;  // 指向下一个资源记录
            }
        }
        else
            need_local_DNS = 1;  // 如果没有找到缓存，设置需要本地DNS标志为1

        if(need_local_DNS == 0 && (response_cache->response->question->qType != question->qType)) {
            need_local_DNS = 1;  // 如果需要本地DNS且缓存中的查询类型不匹配，设置需要本地DNS标志为1
        }
        if(!need_local_DNS)
            printf("Local cache!\n");  // 输出本地缓存信息

        if(need_local_DNS) {  // 如果需要本地DNS
            pthread_t tid;  // 定义线程ID
            char copy_buffer_for_debug[1024];  // 定义调试用缓冲区
            for(int i = 0; i < 1024; i++) {
                copy_buffer_for_debug[i] = buf[i];  // 复制缓冲区内容
            }
            DNSHeader *copy_header_for_debug = malloc(sizeof(DNSHeader));  // 为调试用头部分配内存
            for(int i = 0; i < 12; i++) {
                ((char *)copy_header_for_debug)[i] = ((char *)header)[i];  // 复制头部内容
            }
            pthread_create(&tid, NULL, communicate_with_local_DNS, buf);  // 创建与本地DNS通信的线程
            for(int i = 0; i < 1e6; i++) {  // 循环等待
                if(get_local_success == 1)
                    break;  // 如果成功，跳出循环
                usleep(1);  // 休眠1微秒
            }
            debug_show_local_DNS(copy_header_for_debug, question, serverIP, recv_num, copy_buffer_for_debug, 1, 0);  // 显示本地DNS调试信息
            if(get_local_success == 0) {  // 如果本地DNS通信失败
                (*response)->header = header;  // 设置响应头部
                (*response)->header->flags = 0x8381;  // 设置响应标志位
                (*response)->header->anCnt = 0x0000;  // 设置回答计数为0
                (*response)->header->qdCnt = 0x0100;  // 设置问题计数
                (*response)->header->nsCnt = 0x0000;  // 设置权威记录计数为0
                (*response)->header->arCnt = 0x0000;  // 设置附加记录计数为0
                (*response)->question = question;  // 设置问题
                (*response)->firstRR = NULL;  // 设置资源记录为NULL
                return BLOCK;  // 返回BLOCK
            } else {
                get_local_success = 0;  // 重置本地DNS通信成功标志
            }

            *response_length = parse_response(buf, (*response));  // 解析响应并获取响应长度
            if(cacheRoot == NULL) {
                cacheRoot = getRoot((*response));  // 如果缓存根节点为空，将响应插入根节点
            } else {
                cacheRoot = insertCache(cacheRoot, (*response));  // 否则将响应插入缓存
            }
        } else {
            (*response) = response_cache->response;  // 使用缓存中的响应
            (*response)->header->transID = header->transID;  // 更新事务ID
        }
        if(need_local_DNS)
            debug_show_local_DNS((*response)->header, (*response)->question, serverIP, *response_length, buf, 1, 0);  // 显示本地DNS调试信息

        pthread_mutex_unlock(&mutex_lock);  // 解锁
        return NOT_EXIST;  // 返回NOT_EXIST
    }
    return A;  // 返回A类型
}

// 与本地DNS通信的线程函数
void *communicate_with_local_DNS(void * buffer) {
    char *buf = buffer;  // 将参数转换为字符缓冲区指针
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);  // 创建UDP套接字
    struct sockaddr_in localDNS_addr;  // 定义本地DNS地址结构
    localDNS_addr.sin_family = AF_INET;  // 设置地址族为IPv4
    localDNS_addr.sin_port = htons(53);  // 设置端口号为53
    localDNS_addr.sin_addr.s_addr = inet_addr(serverIP);  // 设置IP地址为服务器IP
    socklen_t len = sizeof(localDNS_addr);  // 获取地址结构大小
    sendto(sockfd, buf, sizeof(char) * 1024, 0, (struct sockaddr *)&localDNS_addr, sizeof(localDNS_addr));  // 发送数据到本地DNS
    recvfrom(sockfd, buf, sizeof(char) * 1024, 0, (struct sockaddr*)&localDNS_addr, &len);  // 从本地DNS接收数据

    get_local_success = 1;  // 设置本地DNS通信成功标志
    close(sockfd);  // 关闭套接字
    return NULL;  // 返回NULL
}

