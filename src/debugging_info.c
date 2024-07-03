#include <stdio.h>    // 包含标准输入输出库
#include <time.h>     // 包含时间库
#include "dns.h"      // 包含自定义的dns头文件
#include <stdlib.h>   // 包含标准库
#include <string.h>   // 包含字符串操作库
#include "packet_parser.h" // 包含自定义的包解析头文件

int debug_num = 1;   // 调试编号，初始值为1
int debug_level = 0; // 调试级别，初始值为0

// 函数声明：转换字节序
void convertEndian(char *data, int size);

// 获取DNS标志位的各个字段
void getFlags(short flags, int *flag_array) {
    convertEndian((char *)(&flags), 2);  // 将flags转换为大端序
    unsigned short mask = 1 << 15;       // 定义掩码，用于提取QR位
    flag_array[0] = (flags & mask) >> 15; // 提取QR位
    mask = 0b1111 << 11;  // 更新掩码，用于提取Opcode位
    flag_array[1] = (flags & mask) >> 11; // 提取Opcode位
    mask = 1 << 10;  // 更新掩码，用于提取AA位
    flag_array[2] = (flags & mask) >> 10; // 提取AA位
    mask = 1 << 9;  // 更新掩码，用于提取TC位
    flag_array[3] = (flags & mask) >> 9;  // 提取TC位
    mask = 1 << 8;  // 更新掩码，用于提取RD位
    flag_array[4] = (flags & mask) >> 8;  // 提取RD位
    mask = 1 << 7;  // 更新掩码，用于提取RA位
    flag_array[5] = (flags & mask) >> 7;  // 提取RA位
    mask = 0b111 << 4;  // 更新掩码，用于提取Z位
    flag_array[6] = (flags & mask) >> 4;  // 提取Z位
    mask = 0b1111;  // 更新掩码，用于提取Rcode位
    flag_array[7] = flags & mask;         // 提取Rcode位
}

// 转换字节序
void convertEndian(char *data, int size) {
    int i = 0;  // 初始化i为0
    int j = size - 1;  // 初始化j为size-1
    while (i < j) {
        char temp = data[i];  // 交换data[i]和data[j]
        data[i] = data[j];
        data[j] = temp;
        i++;
        j--;
    }
}

// 设置调试级别
void set_up(int level) {
    debug_level = level;  // 设置全局调试级别
}

// 显示本地DNS调试信息
void debug_show_local_DNS(DNSHeader *header, DNSQuestion *question, char IP[], int send_num, char *send_buffer, int isSend, int isClient) {
    if (debug_level == 2) {  // 如果调试级别为2
        if (isSend) {
            printf("SEND to %s (%d bytes) ", IP, send_num);  // 显示发送的信息
        } else {
            printf("RECV from %s (%d bytes) ", IP, send_num);  // 显示接收的信息
        }
        if (isClient) {
            printf("[ Client ]\n");  // 显示客户端信息
        } else {
            printf("[ Default DNS Server ]\n");  // 显示默认DNS服务器信息
        }
        for (int i = 0; i < send_num; i++) {
            printf("%02x ", (unsigned char)(send_buffer[i]));  // 显示缓冲区内容
        }
        printf("\n");
        int flag_array[8];
        getFlags(header->flags, flag_array);  // 获取标志位
        char *prompt[] = {"QR", "Opcode", "AA", "TC", "RD", "RA", "Z", "Rcode"};
        for (int i = 0; i < 8; i++) {
            printf("%s: %d\t", prompt[i], flag_array[i]);  // 显示标志位信息
        }
        printf("\n");

        short qdCnt = header->qdCnt;  // 获取问题计数
        convertEndian((char *)(&qdCnt), 2);  // 转换字节序
        printf("Question count: %d\t", qdCnt);  // 显示问题计数

        short anCnt = header->anCnt;  // 获取回答计数
        convertEndian((char *)(&anCnt), 2);  // 转换字节序
        printf("Answer count: %d\t", anCnt);  // 显示回答计数

        short nsCnt = header->nsCnt;  // 获取权威记录计数
        convertEndian((char *)(&nsCnt), 2);  // 转换字节序
        printf("Authority count: %d\t", nsCnt);  // 显示权威记录计数

        short arCnt = header->arCnt;  // 获取附加记录计数
        convertEndian((char *)(&arCnt), 2);  // 转换字节序
        printf("Additional count: %d\t", arCnt);  // 显示附加记录计数

        printf("\n");
        fflush(stdout);  // 刷新输出缓冲区
    }
}

// 显示调试信息
void debug_show(DNSHeader *header, DNSQuestion *question, char IP[], int recv_num, char *rev_buffer) {
    char *query_name = malloc(sizeof(char) * (strlen(question->qName) + 1));  // 为查询名分配内存
    convert_string(question->qName, query_name);  // 转换查询名

    if (debug_level == 1 || debug_level == 2) {  // 如果调试级别为1或2
        printf("\n-------------------------------------------------------\n");

        time_t curtime;
        time(&curtime);  // 获取当前时间
        printf("%d : %s", debug_num++, ctime(&curtime));  // 显示调试编号和当前时间
        short ID = header->transID;  // 获取交易ID
        convertEndian((char *)&ID, 2);  // 转换字节序
        printf("[ID : %02x%02x]\n", *((unsigned char*)&ID + 1), *(unsigned char*)&ID);  // 显示交易ID
        printf("%s", IP);  // 显示IP地址
        printf("\t%s", query_name);  // 显示查询名
        short qType = question->qType;  // 获取查询类型
        convertEndian((char *)&qType, 2);  // 转换字节序
        printf("\tTYPE %d", qType);  // 显示查询类型
        short qClass = question->qClass;  // 获取查询类
        convertEndian((char *)&qClass, 2);  // 转换字节序
        printf("\tCLASS %d", qClass);  // 显示查询类
        printf("\n");
    }
    if (debug_level == 2) {  // 如果调试级别为2
        printf("\nRECV from %s (%d bytes) [ Client ]\n", IP, recv_num);  // 显示接收信息
        for (int i = 0; i < recv_num; i++) {
            printf("%02x ", (unsigned char)(rev_buffer[i]));  // 显示缓冲区内容
        }
        printf("\n");
        int flag_array[8];
        getFlags(header->flags, flag_array);  // 获取标志位
        char *prompt[] = {"QR", "Opcode", "AA", "TC", "RD", "RA", "Z", "Rcode"};
        for (int i = 0; i < 8; i++) {
            printf("%s: %d\t", prompt[i], flag_array[i]);  // 显示标志位信息
        }
        printf("\n");

        short qdCnt = header->qdCnt;  // 获取问题计数
        convertEndian((char *)(&qdCnt), 2);  // 转换字节序
        printf("Question count: %d\t", qdCnt);  // 显示问题计数

        short anCnt = header->anCnt;  // 获取回答计数
        convertEndian((char *)(&anCnt), 2);  // 转换字节序
        printf("Answer count: %d\t", anCnt);  // 显示回答计数

        short nsCnt = header->nsCnt;  // 获取权威记录计数
        convertEndian((char *)(&nsCnt), 2);  // 转换字节序
        printf("Authority count: %d\t", nsCnt);  // 显示权威记录计数

        short arCnt = header->arCnt;  // 获取附加记录计数
        convertEndian((char *)(&arCnt), 2);  // 转换字节序
        printf("Additional count: %d\t", arCnt);  // 显示附加记录计数

        printf("\n");
        fflush(stdout);  // 刷新输出缓冲区
    }
    free(query_name);  // 释放分配的内存
}

// 显示发送信息的调试信息
void debug_show_send_mes(char IP[], int length) {
    if (debug_level == 2) {  // 如果调试级别为2
        printf("SEND to %s (%d bytes)\n", IP, length);  // 显示发送信息
    }
}

