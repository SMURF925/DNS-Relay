#include "dns.h"  // 包含自定义的 dns 头文件
#include <stdlib.h>  // 包含标准库，用于内存分配和释放

// 释放 DNS 问题列表的内存
void free_question_list(DNSQuestion *question_list) {
    DNSQuestion *p = question_list;  // 初始化指针 p 指向问题列表的头节点
    while(p) {  // 循环遍历整个问题列表
        if(p->qName) {  // 如果当前节点的 qName 不为空
            free(p->qName);  // 释放 qName 的内存
            p->qName = NULL;  // 将 qName 置为 NULL
        }
        DNSQuestion *temp = p;  // 临时保存当前节点
        p = p->next;  // 将指针 p 移动到下一个节点
        free(temp);  // 释放当前节点的内存
    }
}

// 释放 DNS 响应的内存
void free_response(Response *response) {
    DNSRR *p = response->firstRR;  // 初始化指针 p 指向响应的第一个资源记录
    while(p) {  // 循环遍历所有资源记录
        free(p->rName);  // 释放 rName 的内存
        free(p->rData);  // 释放 rData 的内存
        DNSRR *temp = p;  // 临时保存当前资源记录节点
        p = p->next;  // 将指针 p 移动到下一个资源记录节点
        free(temp);  // 释放当前资源记录节点的内存
    }
    free(response);  // 释放响应结构体的内存
}
