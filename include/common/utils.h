#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

// 初始化守护进程
void daemon_init();

// 回收僵尸进程
void reap_zombies();

// 错误处理
void handle_error(const char *msg);

#endif // UTILS_H