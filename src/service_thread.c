#include <stdio.h>    // 包含标准输入输出库
#include <sys/types.h>  // 包含系统数据类型定义
#include <sys/socket.h>  // 包含套接字函数定义
#include <arpa/inet.h>  // 包含网络地址转换函数
#include <unistd.h>   // 包含Unix标准函数定义
#include <netinet/in.h>  // 包含Internet地址族定义
#include "dns.h"      // 包含自定义的DNS头文件
#include <stdlib.h>   // 包含标准库，用于内存分配和转换
#include <string.h>   // 包含字符串操作库
#include <ctype.h>    // 包含字符操作函数
#include "packet_parser.h"  // 包含包解析头文件
#include "garbage_collection.h"  // 包含垃圾回收头文件
#include "debugging_info.h"  // 包含调试信息头文件

// 函数声明
void copy_consecutive_bytes(char *src, char* des, int num, int *cpy_cnt);
void copy_string(char *des, char *src, int *cpy_cnt);
void generate_response_packet(int *cpy_cnt, Response *response, DNSHeader* header, DNSQuestion* question, char response_buff[], int status);

// 服务线程函数
void* service_thread(void *arg) {
    int sockfd = *(int *)arg;  // 获取套接字描述符
    while(1) {  // 无限循环处理请求
        struct sockaddr_in cli;  // 定义客户端地址结构
        socklen_t len = sizeof(cli);  // 定义地址长度，recvfrom 会在接收到请求后填充这个字段
        char buf[1024];  // 定义缓冲区
        char *deep_copy_header = malloc(sizeof(char) * 1024);  // 分配内存用于深拷贝的头部
        
        recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&cli, &len);  // 接收数据
        char deep_copy_buf[1024];  // 定义深拷贝缓冲区
        for(int i = 0; i < 1024; i++) {
            deep_copy_buf[i] = buf[i];  // 拷贝数据到深拷贝缓冲区
            deep_copy_header[i] = buf[i];  // 拷贝数据到深拷贝头部
        }

        DNSHeader* header = (DNSHeader*)deep_copy_header;  // 将深拷贝头部转换为DNS头部
        DNSQuestion* question = (DNSQuestion *)malloc(sizeof(DNSQuestion));  // 为DNS问题分配内存
        
        int recv_num = parse_query(buf, header, question);  // 解析查询
        
        char response_buff[2048];  // 定义响应缓冲区
        Response* response = (Response*)malloc(sizeof(Response));  // 为响应分配内存
        
        int cpy_cnt = 0;  // 初始化拷贝计数
        debug_show(header, question, inet_ntoa(cli.sin_addr), recv_num, deep_copy_header);  // 显示调试信息

        int response_length = -1;  // 初始化响应长度
        if(header->flags == 0x0001) {  // 如果是查询请求
            enum TYPE status = normal_handling(&response, header, question, deep_copy_header, &response_length, recv_num);  // 处理请求
            if(status == A || status == AAAA) {  // 如果状态为A或AAAA
                generate_response_packet(&cpy_cnt, response, header, question, response_buff, A);  // 生成响应包
                sendto(sockfd, &response_buff, sizeof(char) * cpy_cnt, 0, (struct sockaddr *)&cli, sizeof(cli));  // 发送响应
            } else if(status == NOT_EXIST) {  // 如果状态为NOT_EXIST
                generate_response_packet(&cpy_cnt, response, header, question, response_buff, NOT_EXIST);  // 生成响应包
                sendto(sockfd, &response_buff, sizeof(char) * cpy_cnt, 0, (struct sockaddr *)&cli, sizeof(cli));  // 发送响应
            } else if(status == BLOCK) {  // 如果状态为BLOCK
                generate_response_packet(&cpy_cnt, response, header, question, response_buff, BLOCK);  // 生成响应包
                sendto(sockfd, &response_buff, sizeof(char) * cpy_cnt, 0, (struct sockaddr *)&cli, sizeof(cli));  // 发送响应
            }
        }
        if(response_length != -1)  // 如果响应长度不为-1
            debug_show_local_DNS((response)->header, (response)->question, inet_ntoa(cli.sin_addr), response_length, response_buff, 1, 1);  // 显示本地DNS调试信息
        else
            debug_show_local_DNS((response)->header, (response)->question, inet_ntoa(cli.sin_addr), cpy_cnt, response_buff, 1, 1);  // 显示本地DNS调试信息
        fflush(stdout);  // 刷新标准输出缓冲区
        //free_question_list(question);  // 释放问题列表
        //free_response(response);  // 释放响应
        
    }
    return NULL;
}

// 拷贝连续字节函数
void copy_consecutive_bytes(char *des, char* src, int num, int *cpy_cnt) {
    for(int i = 0; i < num; i++) {
        des[*cpy_cnt] = src[i];  // 拷贝字节
        (*cpy_cnt)++;  // 增加拷贝计数
    }
}

// 拷贝名称函数
void copy_name(char *des, char *src, int *cpy_cnt) {
    char *p_str = src;
    if((unsigned char)(*p_str) == 0xC0) {  // 如果名称是压缩的
        des[*cpy_cnt] = *p_str;  // 拷贝压缩标志
        p_str++;
        (*cpy_cnt)++;
        des[*cpy_cnt] = *p_str;  // 拷贝压缩指针
        (*cpy_cnt)++;
    } else {
        while(*p_str) {  // 拷贝完整名称
            des[*cpy_cnt] = *p_str;
            p_str++;
            (*cpy_cnt)++;
        }
        des[*cpy_cnt] = 0;  // 末尾添加终止符
        (*cpy_cnt)++;
    }
}

// 拷贝字符串函数
void copy_string(char *des, char *src, int *cpy_cnt) {
    char *p_str = src;
    while(*p_str) {  // 拷贝完整字符串
        des[*cpy_cnt] = *p_str;
        p_str++;
        (*cpy_cnt)++;
    }
    des[*cpy_cnt] = 0;  // 末尾添加终止符
    (*cpy_cnt)++;
}

// 生成响应包函数
void generate_response_packet(int *cpy_cnt, Response *response, DNSHeader* header, DNSQuestion* question, char response_buff[], int status) {
    // 拷贝响应头部
    copy_consecutive_bytes(response_buff, (char *)(response->header), 12, cpy_cnt);
    
    // 拷贝问题名称
    copy_name(response_buff, response->question->qName, cpy_cnt);
    
    // 拷贝问题类型和类
    copy_consecutive_bytes(response_buff, (char *)(&(response->question->qType)), 4, cpy_cnt);
    
    DNSRR *RR_pointer = response->firstRR;
    while(RR_pointer) {  // 遍历资源记录
        // 拷贝资源记录名称
        copy_name(response_buff, RR_pointer->rName, cpy_cnt);
        
        // 拷贝资源记录类型、类、生存时间和数据长度
        copy_consecutive_bytes(response_buff, (char *)(&(RR_pointer->rType)), 10, cpy_cnt);
        
        // 拷贝资源数据
        if(RR_pointer->rType == A) {  // 如果资源记录类型为A
            char * ia;
            struct in_addr* data_addr = malloc(sizeof(struct in_addr));  // 为IPv4地址分配内存
            if(!inet_aton(RR_pointer->rData, data_addr)) {  // 转换IP地址
                ia = RR_pointer->rData;
            } else {
                int ip_address = data_addr->s_addr;
                ia = (char *)(&ip_address);
            }

            for(int i = 0; i < 4; i++) {  // 拷贝IPv4地址
                response_buff[*cpy_cnt] = ia[i];
                (*cpy_cnt)++;
            }
            free(data_addr);  // 释放IPv4地址内存
        } else if(RR_pointer->rType == AAAA) {  // 如果资源记录类型为AAAA
            if(status == NOT_EXIST) {  // 如果状态为NOT_EXIST
                short rdLen = RR_pointer->rdLen;
                convertEndian((char *)&rdLen, 2);  // 转换字节序
                for(int i = 0; i < rdLen; i++) {  // 拷贝IPv6地址
                    response_buff[*cpy_cnt] = RR_pointer->rData[i];
                    (*                    (*cpy_cnt)++;
                }
            } else {
                char *v6 = RR_pointer->rData;  // 获取IPv6地址数据
                unsigned int result[4];  // 定义存储IPv6地址转换结果的数组
                ipv6convert(v6, result);  // 转换IPv6地址

                for(int i = 0; i < 4; i++) {
                    char * ia = (char *)(&result[i]);

                    for(int j = 0; j < 4; j++) {
                        response_buff[*cpy_cnt] = ia[4 - j - 1];  // 按字节拷贝IPv6地址
                        (*cpy_cnt)++;
                    }
                }
            }
        } else {
            short rdLen = RR_pointer->rdLen;
            convertEndian((char *)&rdLen, 2);  // 转换字节序
            for(int i = 0; i < rdLen; i++) {
                response_buff[*cpy_cnt] = RR_pointer->rData[i];  // 拷贝资源数据
                (*cpy_cnt)++;
            }
        }
        RR_pointer = RR_pointer->next;  // 移动到下一个资源记录
    }
}

