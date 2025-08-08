#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <syslog.h>
#include <errno.h>
#include "common/protocol.h"
#include "server/request_handler.h"
#include "common/utils.h"

// 服务端口
#define SERVER_PORT 8080

// 最大客户端数
#define MAX_CLIENTS 10

// 请求处理函数
int handle_client(int sock);

#endif // SERVER_H
