#include "server/server.h"

// 全局变量
volatile sig_atomic_t shutdown_requested = 0;
int server_fd = -1;
int management_fd = -1;
pid_t *child_pids = NULL;
int child_count = 0;
int max_child_count = 0;


// 添加子进程PID
void add_child_pid(pid_t pid)
{
    if (child_count >= max_child_count)
    {
        max_child_count = max_child_count == 0 ? 10 : max_child_count * 2;
        pid_t *new_list = realloc(child_pids, max_child_count * sizeof(pid_t));
        if (!new_list)
        {
            syslog(LOG_ERR, "Failed to allocate memory for child PIDs");
            return;
        }
        child_pids = new_list;
    }
    child_pids[child_count++] = pid;
}

// 终止所有子进程
void terminate_all_children()
{
    syslog(LOG_INFO, "Terminating all child processes");

    // 发送SIGTERM给所有子进程
    for (int i = 0; i < child_count; i++)
    {
        if (child_pids[i] > 0)
        {
            kill(child_pids[i], SIGTERM);
        }
    }

    // 等待所有子进程退出
    int still_running = child_count;
    int timeout = 5; // 5秒超时

    while (still_running > 0 && timeout > 0)
    {
        pid_t pid;
        int status;

        // 非阻塞等待子进程退出
        while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
        {
            for (int i = 0; i < child_count; i++)
            {
                if (child_pids[i] == pid)
                {
                    child_pids[i] = -1; // 标记为已退出
                    still_running--;
                    syslog(LOG_INFO, "Child process %d exited", pid);
                    break;
                }
            }
        }

        if (still_running > 0)
        {
            sleep(1);
            timeout--;
        }
    }

    // 强制终止剩余子进程
    if (still_running > 0)
    {
        syslog(LOG_INFO, "Forcefully terminating %d remaining child processes", still_running);
        for (int i = 0; i < child_count; i++)
        {
            if (child_pids[i] > 0)
            {
                kill(child_pids[i], SIGKILL);
            }
        }

        // 等待所有子进程退出
        while (waitpid(-1, NULL, 0) > 0);
    }

    free(child_pids);
    child_pids = NULL;
    child_count = 0;
    max_child_count = 0;
}

int create_server_socket(struct sockaddr_in *address, int addrlen)
{
	// 创建服务Socket
	int svr_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (svr_fd == 0)
    {
        handle_error("Socket creation failed");
    }

    // 设置socket选项
    int opt = 1;
    if (setsockopt(svr_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        handle_error("Setsockopt failed");
    }

    address->sin_family = AF_INET;
    address->sin_addr.s_addr = INADDR_ANY;
    address->sin_port = htons(SERVER_PORT);

    // 绑定socket
    if (bind(svr_fd, (struct sockaddr *)address, addrlen) < 0)
    {
        handle_error("Bind failed");
    }

    // 监听
    if (listen(svr_fd, MAX_CLIENTS) < 0)
    {
        handle_error("Listen failed");
    }

    syslog(LOG_INFO, "Server listening on port %d", SERVER_PORT);

	return svr_fd;
}

// 创建管理Socket
int create_management_socket()
{
    int mgmt_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (mgmt_fd < 0)
    {
        syslog(LOG_ERR, "Management socket creation failed");
        return -1;
    }

    // 设置socket选项
    int opt = 1;
    if (setsockopt(mgmt_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        syslog(LOG_ERR, "Management setsockopt failed");
        close(mgmt_fd);
        return -1;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(MANAGEMENT_PORT);

    // 绑定socket
    if (bind(mgmt_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        syslog(LOG_ERR, "Management bind failed");
        close(mgmt_fd);
        return -1;
    }

    // 监听
    if (listen(mgmt_fd, 1) < 0)
    {
        syslog(LOG_ERR, "Management listen failed");
        close(mgmt_fd);
        return -1;
    }

    syslog(LOG_ERR, "Management interface listening on port %d", MANAGEMENT_PORT);
    return mgmt_fd;
}

int main()
{
    daemon_init();				//派生出子进程的子进程，将其完全打造为deamon，关闭一切的文件描述符和输入输出等。

    syslog(LOG_INFO, "Server daemon started");

	struct sockaddr_in address;
	int addrlen = sizeof(address);

	server_fd = create_server_socket(&address, addrlen);	// 创建服务Socket
	if (server_fd < 0)
    {
        syslog(LOG_ERR, "server interface not available");
    }

    management_fd = create_management_socket();				// 创建管理Socket
    if (management_fd < 0)
    {
        syslog(LOG_ERR, "Management interface not available");
    }

    // 主循环
    while (!shutdown_requested)
    {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        if (management_fd != -1)
        {
            FD_SET(management_fd, &readfds);
        }

        int max_fd = server_fd;
        if (management_fd != -1 && management_fd > max_fd)
        {
            max_fd = management_fd;
        }

        // 设置超时1秒
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int ret = select(max_fd + 1, &readfds, NULL, NULL, &tv);
        if (ret < 0)
        {
            if (errno == EINTR && shutdown_requested)
            {
                break; // 信号中断且需要关闭
            }
            syslog(LOG_ERR, "Select error: %s", strerror(errno));
            continue;
        }

        // 检查管理Socket
        if (management_fd != -1 && FD_ISSET(management_fd, &readfds))
        {
            int mgmt_conn = accept(management_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
            if (mgmt_conn < 0)
            {
                syslog(LOG_ERR, "Management accept failed: %s", strerror(errno));
            }
            else
            {
                char cmd[32];
                ssize_t bytes_read = read(mgmt_conn, cmd, sizeof(cmd) - 1);
                if (bytes_read > 0)
                {
                    cmd[bytes_read] = '\0';
                    if (strcmp(cmd, "stop") == 0)
                    {
                        syslog(LOG_ERR, "Received shutdown command via management interface");
                        shutdown_requested = 1;
                        const char *resp = "Stopping server\n";
                        if (write(mgmt_conn, resp, strlen(resp)) == -1)
						{
							syslog(LOG_ERR, "Get result error");
							return -1;
			            }
                    }
                }
                close(mgmt_conn);
            }
        }

        // 检查服务Socket
        if (FD_ISSET(server_fd, &readfds))
        {
            int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
            if (new_socket < 0)
            {
                if (errno == EINTR && shutdown_requested)
                {
                    break; // 信号中断且需要关闭
                }
                syslog(LOG_ERR, "Accept failed: %s", strerror(errno));
                continue;
            }

            syslog(LOG_INFO, "New connection from %s", inet_ntoa(address.sin_addr));

            // 创建子进程处理请求
            pid_t pid = fork();
            if (pid < 0)
            {
                syslog(LOG_ERR, "Fork failed");
                close(new_socket);
                continue;
            }

            if (pid == 0) // 子进程
            {
                close(server_fd); // 关闭监听socket
                if (management_fd != -1)
                {
                    close(management_fd); // 关闭管理socket
                }

                // 处理客户端请求
                if (handle_client(new_socket) == -1)
                {
					syslog(LOG_ERR, "handle client socket error!");
                }

				close(new_socket);
                syslog(LOG_INFO, "Client connection closed");
                exit(EXIT_SUCCESS);
            }
            else
            { // 父进程
                close(new_socket); // 关闭客户端socket
                add_child_pid(pid); // 记录子进程PID
            }
        }

        // 回收僵尸进程
        reap_zombies();
    }

    // 优雅关闭阶段
    syslog(LOG_INFO, "Shutting down server...");

    // 终止所有子进程
    terminate_all_children();

    // 关闭socket
    if (server_fd != -1)
    {
        close(server_fd);
    }
    if (management_fd != -1)
    {
        close(management_fd);
    }

    syslog(LOG_INFO, "Server shutdown complete");

    return EXIT_SUCCESS;
}
