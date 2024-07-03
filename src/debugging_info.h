#ifndef debugging_info_h  // 如果未定义 debugging_info_h 宏
#define debugging_info_h  // 定义 debugging_info_h 宏

// 函数声明
extern void set_up(int level);  // 声明外部函数 set_up，用于设置调试级别
extern void debug_show(DNSHeader *header, DNSQuestion *question, char IP[], int recv_num, char *rev_buffer);  // 声明外部函数 debug_show，用于显示调试信息
extern void convertEndian(char *data, int size);  // 声明外部函数 convertEndian，用于转换字节序
extern void debug_show_send_mes(char IP[], int length);  // 声明外部函数 debug_show_send_mes，用于显示发送信息的调试信息
extern void debug_show_local_DNS(DNSHeader *header, DNSQuestion *question, char IP[], int send_num, char *send_buffer, int isSend, int isClient);  // 声明外部函数 debug_show_local_DNS，用于显示本地DNS调试信息

#endif /* debugging_info_h */  // 结束条件编译块
