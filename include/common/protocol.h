#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

// 命令类型
typedef enum
{
	CMD_STOP = 0x00,		// 关闭所有服务
    CMD_ECHO = 0x01,       // 回显测试
    CMD_GET_INFO = 0x02,   // 获取系统信息
    CMD_EXECUTE = 0x03,    // 执行命令
    CMD_CALC = 0x04,    // 关闭服务端
} CommandType;

// 请求头
typedef struct
{
    uint8_t version;        // 协议版本
    CommandType cmd;        // 命令类型
    uint16_t data_len;      // 数据长度
} RequestHeader;

// 响应头
typedef struct
{
    uint8_t version;        // 协议版本
    uint8_t status;         // 状态码 (0=成功, 其他=错误码)
    uint16_t data_len;      // 数据长度
} ResponseHeader;

// 最大数据长度
#define MAX_DATA_LEN 4096
#define MANAGEMENT_PORT 8081 // 管理端口


#endif // PROTOCOL_H
