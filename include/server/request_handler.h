#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include "common/protocol.h"

// 处理客户端请求
int handle_client(int sock);

#endif // REQUEST_HANDLER_H
