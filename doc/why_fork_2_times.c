#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#define LOG_FILE "/tmp/sighup_debug.log"

void log_message(const char *message) {
    int fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) return;
    
    time_t now = time(NULL);
    char buf[1024];
    int len = snprintf(buf, sizeof(buf), "[%ld] [PID=%d] [PPID=%d] [PGID=%d] [SID=%d] %s\n",
                      now, getpid(), getppid(), getpgid(0), getsid(0), message);
    write(fd, buf, len);
    close(fd);
}

void handler(int sig) {
    log_message("Received SIGHUP signal");
    // 不要立即退出，以便记录更多信息
}

int main() {
    // 确保日志文件存在
    int fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd != -1) close(fd);
    
    log_message("Program started");
    
    // 注册信号处理器
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    
    if (sigaction(SIGHUP, &sa, NULL) == -1) {
        log_message("Failed to register SIGHUP handler");
    }
    
    pid_t pid = fork();
    if (pid < 0) {
        log_message("Fork failed");
        return 1;
    }
    
    if (pid > 0) {
        log_message("Parent exiting");
        return 0;
    }
    
    // 子进程继续
    log_message("Child process running");
    
    // 打印进程信息到控制台
    printf("Child PID: %d\n", getpid());
    printf("Process group: %d\n", getpgid(0));
    printf("Session ID: %d\n", getsid(0));
    fflush(stdout);
    
    // 主循环
    int counter = 0;
    while (1) {
        sleep(5);
        counter++;
        
        char msg[256];
        snprintf(msg, sizeof(msg), "Still running, counter=%d", counter);
        log_message(msg);
        
        // 检查父进程是否改变
        if (getppid() == 1) {
            log_message("Parent process changed to init (PPID=1)");
        }
    }
    
    return 0;
}
