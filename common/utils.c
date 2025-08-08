#include "common/utils.h"

void daemon_init()
{
    pid_t pid = fork();

    if (pid < 0)
	{
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid > 0)
	{
        exit(EXIT_SUCCESS); // 父进程退出
    }

    // 子进程成为会话组长
    if (setsid() < 0)
	{
        perror("setsid failed");
        exit(EXIT_FAILURE);
    }

    // 再次fork确保不是会话组长
    pid = fork();
    if (pid < 0)
	{
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid > 0)
	{
        exit(EXIT_SUCCESS); // 父进程退出
    }


    // 设置工作目录
    if (chdir("/") == -1)
    {
		exit(EXIT_SUCCESS);	// 子进程也自己退出
    }

	umask(0);

    // 关闭所有打开的文件描述符
    for (int fd = sysconf(_SC_OPEN_MAX); fd >= 0; fd--)
	{
        close(fd);
    }

    // 重定向标准IO
    int null_fd = open("/dev/null", O_RDWR); // stdin
    dup2(null_fd, STDIN_FILENO);
	dup2(null_fd, STDOUT_FILENO);
	dup2(null_fd, STDERR_FILENO);

    // 初始化syslog
    openlog("cs_daemon_server", LOG_PID, LOG_DAEMON);

	// 忽略终端信号
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
}

void reap_zombies()
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handle_error(const char *msg)
{
    syslog(LOG_ERR, "%s", msg);
    exit(EXIT_FAILURE);
}
