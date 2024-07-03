#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <time.h>

#define NUM_THREADS 10 // 线程数量
#define REQUESTS_PER_THREAD 10 // 每个线程发送的请求数量

// 发送 DNS 查询请求的函数
void* send_dns_query(void* arg) {
    char* hostname = (char*) arg; // 要查询的主机名
    struct addrinfo hints, *res;
    int err;

    memset(&hints, 0, sizeof(hints)); // 初始化 addrinfo 结构体
    hints.ai_family = AF_INET; // 设置地址族为 IPv4
    hints.ai_socktype = SOCK_STREAM; // 设置套接字类型为流

    // 循环发送 DNS 查询请求
    for (int i = 0; i < REQUESTS_PER_THREAD; i++) {
        if ((err = getaddrinfo(hostname, NULL, &hints, &res)) != 0) { // 执行 DNS 查询
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err)); // 查询失败时打印错误信息
        } else {
            printf("Query %d: Got address for %s\n", i + 1, hostname); // 查询成功时打印信息
            freeaddrinfo(res); // 释放 addrinfo 结构体
        }
        usleep(100000); // 可选：在每次请求之间添加一个小的延迟，避免过度请求
    }

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS]; // 线程数组
    char* hostname = "www.example.com"; // 要查询的主机名
    clock_t start_time, end_time;

    start_time = clock(); // 记录开始时间

    // 创建线程
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, send_dns_query, (void*)hostname) != 0) { // 创建线程
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    // 等待所有线程完成
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    end_time = clock(); // 记录结束时间

    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC; // 计算总耗时
    printf("Test completed in %.2f seconds\n", time_spent); // 打印总耗时

    return 0;
}
