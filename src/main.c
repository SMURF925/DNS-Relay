#include <stdio.h>    
#include <sys/types.h>  
#include <sys/socket.h>  
#include <arpa/inet.h>  
#include <unistd.h>  
#include "dns.h"  
#include <stdlib.h>  
#include <string.h>  
#include <ctype.h>  
#include <pthread.h>  
#include "AVL.h"  
#include "debugging_info.h"  
#include "AVL_Cache.h"  
#include <netdb.h> // 包含 struct addrinfo, getaddrinfo, gai_strerror 和 freeaddrinfo 的声明


#define MAX_FILE_LINE_WIDTH 128  
#define SERVICE_THREAD_NUM 5  
#define NUM_TEST_THREADS 10  // 测试线程数量
#define REQUESTS_PER_THREAD 10  // 每个测试线程的请求数量

pthread_mutex_t mutex_lock;  
char *serverIP;  

void* send_dns_query(void* arg);  // 声明发送 DNS 查询请求的函数

int main(int argc, const char * argv[]) {
    pthread_mutex_init(&mutex_lock, NULL);  
    printf("Starting DNS relay......\n");  
    printf("Mutex lock initialized.\n");  
    cacheRoot = NULL;  
    serverIP = malloc(sizeof(char) * 100);  
    if(strcmp(argv[1], "-d") == 0)  
    {
        set_up(1);  
        strcpy(serverIP, argv[2]);  
    }
    else if(strcmp(argv[1], "-dd") == 0)  
    {
        set_up(2);  
        strcpy(serverIP, argv[2]);  
    }
    else
    {
        set_up(0);  
        strcpy(serverIP, argv[1]);  
    }
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);  
    struct sockaddr_in addr;  
    addr.sin_family = AF_INET;  
    addr.sin_port = htons(53);  
    addr.sin_addr.s_addr = htonl(INADDR_ANY);  

    char file_name[] = "./database.txt";  
    FILE *fin;  
    char *one_line;  
    char *domain = NULL;  
    one_line = (char*)malloc(MAX_FILE_LINE_WIDTH * sizeof(char));  
    char *IP = NULL;  
    int f = 0;  
    root = (AVL*)malloc(sizeof(AVL));  
    if((fin = fopen(file_name, "r")) == NULL)  
    {
        printf("can not open file %s !\n", file_name);  
    }
    char *type = NULL;  

    while(fgets(one_line, MAX_FILE_LINE_WIDTH, fin) != NULL)  
    {
        int first = 1;  
        int len = strlen(one_line);  
        if(one_line[len - 1] == '\n')  
        {
            one_line[len - 1] = 0;  
        }

        for(int i = 0; i < len; i++)  
        {
            if(one_line[i] == ' ')  
            {
                one_line[i] = 0;  
                domain = one_line;  

                if(first)  
                {
                    type = &one_line[i + 1];  
                    first = 0;  
                }
                else
                {
                    IP = &one_line[i + 1];  
                    one_line[len] = 0;  
                }
            }
        }

        if(f == 0) {  
            f = 1;  
            root->domain = malloc(sizeof(char) * sizeof(domain));  
            root->type = malloc(sizeof(char) * sizeof(type));  
            root->ip = malloc(sizeof(char) * sizeof(IP));  
            strcpy(root->domain, domain);  
            strcpy(root->type, type);  
            strcpy(root->ip, IP);  
            root->next = NULL;  
            root->lchild = NULL;  
            root->rchild = NULL;  
            root->height = 0;  
        }
        else {
            root = insert(root, domain, type, IP);  
        }

    }

    int ret = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));  
    if(ret < 0) {
        printf("bind error\n");  
        return -1;  
    }
    printf("AVL tree build successfully.\n");  
    int num_of_threads = SERVICE_THREAD_NUM;  
    printf("Create %d service threads.\n", num_of_threads);  

    fflush(stdout);  
    free(one_line);  
    pthread_t service_tid[SERVICE_THREAD_NUM];  

    for(int i = 0; i < SERVICE_THREAD_NUM; i++)  
    {
        int arg = sockfd;  
        pthread_create(&service_tid[i], NULL, service_thread, &arg);  
    }

    // 添加压力测试代码
    pthread_t test_threads[NUM_TEST_THREADS];  
    char* test_hostname = "www.bupt.edu.cn";  // 要查询的主机名，可以更改为你希望测试的域名
    clock_t start_time, end_time;

    start_time = clock();  // 记录开始时间

    // 创建测试线程
    for (int i = 0; i < NUM_TEST_THREADS; i++) {
        if (pthread_create(&test_threads[i], NULL, send_dns_query, (void*)test_hostname) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    // 等待所有测试线程完成
    for (int i = 0; i < NUM_TEST_THREADS; i++) {
        pthread_join(test_threads[i], NULL);
    }

    end_time = clock();  // 记录结束时间

    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;  // 计算总耗时
    printf("Test completed in %.2f seconds\n", time_spent);  // 打印总耗时

    // 等待服务线程结束
    for (int i = 0; i < SERVICE_THREAD_NUM; i++) {
        pthread_join(service_tid[i], NULL);
    }

    return 0;
}

// 发送 DNS 查询请求的函数
void* send_dns_query(void* arg) {
    char* hostname = (char*) arg;  // 要查询的主机名
    struct addrinfo hints, *res;
    int err;

    memset(&hints, 0, sizeof(hints));  // 初始化 addrinfo 结构体
    hints.ai_family = AF_INET;  // 设置地址族为 IPv4
    hints.ai_socktype = SOCK_STREAM;  // 设置套接字类型为流

    // 循环发送 DNS 查询请求
    for (int i = 0; i < REQUESTS_PER_THREAD; i++) {
        if ((err = getaddrinfo(hostname, NULL, &hints, &res)) != 0) {  // 执行 DNS 查询
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));  // 查询失败时打印错误信息
        } else {
            printf("Query %d: Got address for %s\n", i + 1, hostname);  // 查询成功时打印信息
            freeaddrinfo(res);  // 释放 addrinfo 结构体
        }
        usleep(100000);  // 可选：在每次请求之间添加一个小的延迟，避免过度请求
    }

    return NULL;
}
