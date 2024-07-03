#include <stdio.h>   // 包含标准输入输出库
#include <string.h>  // 包含字符串操作库
#include <stdlib.h>  // 包含标准库，用于内存分配和转换

// 反转字符串函数
void strrev(char str[]) {
    int i = 0;  // 初始化左指针 i 为 0
    int j = strlen(str) - 1;  // 初始化右指针 j 为字符串长度减 1
    while(i < j) {  // 当左指针小于右指针时，交换字符
        char temp = str[i];  // 保存左指针字符
        str[i] = str[j];  // 将右指针字符赋值给左指针位置
        str[j] = temp;  // 将保存的字符赋值给右指针位置
        i++;  // 左指针右移
        j--;  // 右指针左移
    }
}

// 转换 IPv6 地址函数，将 IPv6 地址字符串转换为 4 个 32 位无符号整数
int ipv6convert(const char *point, unsigned int result[4]) {
    for (int i = 0; i < 4; i++) {
        result[i] = 0;  // 初始化结果数组
    }
    char buf[40] = {0}, revbuf[40], p = 0, q = 0;  // 定义缓冲区和指针
    strcpy(buf, point);  // 复制输入字符串到缓冲区 buf
    strcpy(revbuf, point);  // 复制输入字符串到缓冲区 revbuf
    strrev(revbuf);  // 反转 revbuf 中的字符串
    buf[strlen(point)] = ':';  // 在 buf 末尾添加一个冒号
    revbuf[strlen(point)] = ':';  // 在 revbuf 末尾添加一个冒号
    for(int i = 0; i < 8; i++) {  // 循环 8 次，每次处理一个 16 位块
        q = strchr(buf + p, ':') - buf;  // 找到下一个冒号的位置
        buf[q] = '\0';  // 将冒号替换为字符串结束符
        if (i % 2 == 0) {
            result[i / 2] = 0;  // 初始化结果数组的当前元素
            result[i / 2] = strtol(buf + p, NULL, 16) << 16;  // 将当前块的前 16 位转换为整数并左移 16 位
        } else {
            result[i / 2] += strtol(buf + p, NULL, 16);  // 将当前块的后 16 位转换为整数并加到结果数组中
        }
        p = q + 1;  // 更新指针 p，跳过冒号
        // 如果发现 "::"，则需要处理反向缓冲区
        if (buf[p] == ':') {
            p = q = 0;  // 重置指针 p 和 q
            for (int j = 7; j > i; j--) {  // 从反向缓冲区的末尾开始处理
                q = strchr(revbuf + p, ':') - revbuf;  // 找到下一个冒号的位置
                revbuf[q] = '\0';  // 将冒号替换为字符串结束符
                strrev(revbuf + p);  // 反转子字符串
                if (j % 2 == 0) {
                    result[j / 2] += strtol(revbuf + p, NULL, 16) << 16;  // 将当前块的前 16 位转换为整数并左移 16 位
                } else {
                    result[j / 2] = 0;  // 初始化结果数组的当前元素
                    result[j / 2] += strtol(revbuf + p, NULL, 16);  // 将当前块的后 16 位转换为整数并加到结果数组中
                }
                p = q + 1;  // 更新指针 p，跳过冒号
                if (revbuf[p] == ':') {
                    break;  // 如果发现冒号，跳出循环
                }
            }
            break;  // 跳出外部循环
        }
    }
    return 1;  // 返回成功标志
}
