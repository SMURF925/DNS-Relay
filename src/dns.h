#ifndef dns_h  // 如果未定义 dns_h 宏
#define dns_h  // 定义 dns_h 宏

// 枚举类型 TYPE，表示不同的DNS记录类型和特殊状态
enum TYPE {
    AAAA = 0x1c00,  // AAAA 记录类型，对应 IPv6 地址
    A = 0x0100,     // A 记录类型，对应 IPv4 地址
    BLOCK = -1,     // 表示阻止的状态
    NOT_EXIST = -2  // 表示不存在的状态
};

// 定义一个 DNSHeader 结构体，用于表示 DNS 报文的头部
typedef struct DNSHeader {
    unsigned short transID;  // 交易 ID
    unsigned short flags;    // 标志位
    unsigned short qdCnt;    // 问题计数
    unsigned short anCnt;    // 回答计数
    unsigned short nsCnt;    // 权威记录计数
    unsigned short arCnt;    // 附加记录计数
} DNSHeader;

// 定义一个 DNSRR 结构体，用于表示 DNS 资源记录
typedef struct DNSRR {
    char *rName;            // 资源记录名称
    short rType;            // 资源记录类型
    short rClass;           // 资源记录类
    int ttl;                // 生存时间
    short rdLen;            // 资源数据长度
    char *rData;            // 资源数据
    struct DNSRR *next;     // 指向下一个资源记录的指针
} DNSRR;

// 定义一个 DNSQuestion 结构体，用于表示 DNS 问题
typedef struct DNSQuestion {
    char *qName;            // 问题名称
    short qType;            // 问题类型
    short qClass;           // 问题类
    struct DNSQuestion *next;  // 指向下一个问题的指针
} DNSQuestion;

// 定义一个 Response 结构体，用于表示 DNS 响应
typedef struct Response {
    DNSHeader *header;      // 指向 DNS 报文头部的指针
    DNSQuestion *question;  // 指向 DNS 问题的指针
    DNSRR *firstRR;         // 指向第一个资源记录的指针
} Response;

// 函数声明
extern void copy_string(char *des, char *src, int *cpy_cnt);  // 声明外部函数 copy_string，用于复制字符串
extern void* service_thread(void *arg);  // 声明外部函数 service_thread，用于处理服务线程
extern enum TYPE normal_handling(Response **response, DNSHeader *header, DNSQuestion *question, char buf[], int *response_length, int recv_num);  // 声明外部函数 normal_handling，用于处理正常的DNS请求
extern enum TYPE search_database(char name[], char *resultIP[], enum TYPE requestType, int *resnum);  // 声明外部函数 search_database，用于在数据库中搜索DNS记录
extern int ipv6convert(const char *point, unsigned int result[4]);  // 声明外部函数 ipv6convert，用于转换IPv6地址

#endif /* dns_h */  // 结束条件编译块
