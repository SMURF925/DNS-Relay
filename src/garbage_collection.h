#ifndef garbage_collcation_h  // 如果未定义 garbage_collcation_h 宏
#define garbage_collcation_h  // 定义 garbage_collcation_h 宏

// 声明外部函数 free_question_list，用于释放 DNS 问题列表的内存
extern void free_question_list(DNSQuestion *question_list);

// 声明外部函数 free_response，用于释放 DNS 响应的内存
extern void free_response(Response *response);

#endif /* garbage_collcation_h */  // 结束条件编译块
