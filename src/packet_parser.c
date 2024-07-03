#include <stdlib.h>  // 包含标准库，用于内存分配和转换
#include <string.h>  // 包含字符串操作库
#include "dns.h"     // 包含自定义的DNS头文件
#include <stdio.h>   // 包含标准输入输出库
#include "debugging_info.h"  // 包含调试信息头文件

// 将DNS格式的字符串转换为标准格式
void convert_string(char src[], char des[]) {
    int resnum = src[0];  // 获取第一个字符，表示段长度
    int i;
    int len = strlen(src);  // 获取源字符串长度
    for(i = 0; i < len - 1; i++) {  // 遍历源字符串
        if(resnum == 0) {  // 如果当前段长度为0
            des[i] = '.';  // 在目标字符串中插入一个点
            resnum = src[i + 1];  // 获取下一个段长度
        } else {
            des[i] = src[i + 1];  // 复制字符到目标字符串
            resnum--;  // 减少当前段的剩余长度
        }
    }
    des[i] = 0;  // 在目标字符串末尾添加终止符
}

// 解析DNS查询部分
int parse_query(char packet[], DNSHeader* header, DNSQuestion* question) {
    char *buf = packet;  // 指向包的缓冲区
    short times = header->qdCnt;  // 获取问题计数
    convertEndian((char *)&times, 2);  // 转换字节序
    int pos = 12;  // 设置初始位置，跳过DNS头部
    for(int i = 0; i < times; i++) {  // 遍历每一个问题
        char *temp = malloc(sizeof(char) * 100);  // 分配内存用于保存问题名称
        strcpy(temp, &buf[pos]);  // 复制问题名称
        pos += 1 + strlen(temp);  // 更新位置
        question->qName = temp;  // 设置问题名称
        question->qType = *(short *)(&buf[pos]);  // 获取问题类型
        pos += 2;  // 更新位置
        question->qClass = *(short *)(&buf[pos]);  // 获取问题类
        pos += 2;  // 更新位置
        
        if(i == times - 1)
            question->next = NULL;  // 设置问题链表末尾
        else {
            question->next = malloc(sizeof(DNSQuestion));  // 为下一个问题分配内存
            question = question->next;  // 移动到下一个问题
        }
    }
    return pos;  // 返回解析结束位置
}

// 解析DNS响应部分
int parse_response(char packet[], Response* response) {
    char * buf = packet;  // 指向包的缓冲区
    response->question = malloc(sizeof(DNSQuestion));  // 分配内存用于保存问题
    response->header = (DNSHeader *)packet;  // 指向DNS头部
    int pos = parse_query(packet, response->header, response->question);  // 解析查询部分并获取结束位置
    short anCnt = response->header->anCnt;  // 获取回答计数
    short nsCnt = response->header->nsCnt;  // 获取权威记录计数
    short arCnt = response->header->arCnt;  // 获取附加记录计数
    convertEndian((char *)&anCnt, sizeof(anCnt));  // 转换字节序
    convertEndian((char *)&nsCnt, sizeof(nsCnt));  // 转换字节序
    convertEndian((char *)&arCnt, sizeof(arCnt));  // 转换字节序
    int times = anCnt + nsCnt + arCnt;  // 总记录数
    response->firstRR = malloc(sizeof(DNSRR));  // 分配内存用于第一个资源记录
    DNSRR* p = response->firstRR;  // 指向第一个资源记录
    for(int i = 0; i < times; i++) {  // 遍历每一个资源记录
        p->rName = malloc(sizeof(char) * 100);  // 分配内存用于保存资源记录名称
        unsigned char prefix = buf[pos];  // 获取前缀
        if(prefix == 0xC0) {  // 如果前缀是0xC0，表示是压缩的名称
            p->rName[0] = 0xC0;  // 设置压缩标志
            p->rName[1] = buf[pos + 1];  // 设置压缩指针
            pos += 2;  // 更新位置
        } else {
            strcpy(p->rName, &buf[pos]);  // 复制资源记录名称
            pos += 1 + strlen(p->rName);  // 更新位置
        }
        p->rType = *(short *)(&buf[pos]);  // 获取资源记录类型
        pos += 2;  // 更新位置
        p->rClass = *(short *)(&buf[pos]);  // 获取资源记录类
        pos += 2;  // 更新位置
        p->ttl = *(int *)(&buf[pos]);  // 获取生存时间
        pos += 4;  // 更新位置
        p->rdLen = *(short *)(&buf[pos]);  // 获取资源数据长度
        short rdLen = p->rdLen;
        convertEndian((char *)&rdLen, sizeof(rdLen));  // 转换字节序
        pos += 2;  // 更新位置

        p->rData = malloc(sizeof(char) * rdLen);  // 分配内存用于保存资源数据
        for(int j = 0; j < rdLen; j++) {  // 复制资源数据
            p->rData[j] = buf[pos];
            pos++;
        }

        if(i == times - 1)
            p->next = NULL;  // 设置资源记录链表末尾
        else {
            p->next = malloc(sizeof(DNSRR));  // 为下一个资源记录分配内存
            p = p->next;  // 移动到下一个资源记录
        }
    }
    return pos;  // 返回解析结束位置
}
