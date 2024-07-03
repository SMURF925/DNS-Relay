#ifndef packet_parser_h  // 如果未定义 packet_parser_h 宏
#define packet_parser_h  // 定义 packet_parser_h 宏

// 声明外部函数 parse_query，用于解析DNS查询
extern int parse_query(char packet[], DNSHeader* header, DNSQuestion* question);

// 声明外部函数 parse_response，用于解析DNS响应
extern int parse_response(char packet[], Response* response);

// 声明外部函数 convert_string，用于转换DNS格式字符串
extern void convert_string(char src[], char des[]);

#endif /* packet_parser_h */  // 结束条件编译块
