#include "client/client.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

int send_request(int sock, CommandType cmd, const char* data)
{
    RequestHeader req_header;
    ResponseHeader resp_header;
    char buffer[MAX_DATA_LEN] = {0};

    // 准备请求头
    req_header.version = 1;
    req_header.cmd = cmd;
    req_header.data_len = data ? strlen(data) : 0;

    // 发送请求头
    if (write(sock, &req_header, sizeof(RequestHeader)) != sizeof(RequestHeader))
    {
        perror("Failed to send request header");
        return -1;
    }

    // 发送请求数据
    if (req_header.data_len > 0)
    {
        if (write(sock, data, req_header.data_len) != req_header.data_len)
        {
            perror("Failed to send request data");
            return -1;
        }
    }

    // 接收响应头
    if (read(sock, &resp_header, sizeof(ResponseHeader)) != sizeof(ResponseHeader))
    {
        perror("Failed to receive response header");
        return -1;
    }

    // 检查状态
    if (resp_header.status != 0)
    {
        fprintf(stderr, "Error response: status 0x%02x\n", resp_header.status);
        return -1;
    }

    // 接收响应数据
    if (resp_header.data_len > 0)
    {
        ssize_t bytes_read = read(sock, buffer, resp_header.data_len);
        if (bytes_read != resp_header.data_len)
        {
            perror("Incomplete response data");
            return -1;
        }
        buffer[resp_header.data_len] = '\0';
        printf("%s", buffer);
    }

    return 0;
}

int send_shutdown_command(void)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Socket creation failed");
        return -1;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(MANAGEMENT_PORT);

    // 转换IP地址
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0)
    {
        perror("Invalid address");
        close(sock);
        return -1;
    }

    // 连接管理接口
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connection to management interface failed");
        close(sock);
        return -1;
    }

    // 发送关机命令
    const char *cmd = "stop";
    if (write(sock, cmd, strlen(cmd)) != (ssize_t)strlen(cmd))
    {
        perror("Failed to send shutdown command");
        close(sock);
        return -1;
    }

    // 读取响应
    char buffer[256];
    ssize_t bytes_read = read(sock, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0)
    {
        buffer[bytes_read] = '\0';
        printf("%s", buffer);
    }

    close(sock);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <command> [arguments]\n", argv[0]);
        fprintf(stderr, "Available commands:\n");
        fprintf(stderr, "  echo <text>      - Echo text\n");
        fprintf(stderr, "  info             - Get server info\n");
        fprintf(stderr, "  exec <command>   - Execute shell command\n");
        fprintf(stderr, "  shutdown         - Shutdown server\n");
		fprintf(stderr, "  stop             - Stop server\n");
        return EXIT_FAILURE;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);

    // 转换IP地址
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0)
    {
        perror("Invalid address");
        close(sock);
        return EXIT_FAILURE;
    }

    // 连接服务器
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connection failed");
        close(sock);
        return EXIT_FAILURE;
    }

    int result = 0;

    // 解析命令
    if (strcmp(argv[1], "echo") == 0 && argc > 2)
    {
        result = send_request(sock, CMD_ECHO, argv[2]);
    }
    else if (strcmp(argv[1], "info") == 0)
    {
        result = send_request(sock, CMD_GET_INFO, NULL);
    }
    else if (strcmp(argv[1], "exec") == 0 && argc > 2)
    {
        // 合并所有参数为单个命令字符串
        char command[1024] = {0};
        for (int i = 2; i < argc; i++)
        {
            strcat(command, argv[i]);
            if (i < argc - 1)
                strcat(command, " ");
        }
        result = send_request(sock, CMD_EXECUTE, command);
    }
    else if (strcmp(argv[1], "calc") == 0 && argc > 2)
    {
		// 合并所有参数为单个命令字符串
        char command[1024] = {0};
        for (int i = 2; i < argc; i++)
        {
            strcat(command, argv[i]);
            if (i < argc - 1)
                strcat(command, " ");
        }
        result = send_request(sock, CMD_CALC, command);
    }
	else if (strcmp(argv[1], "stop") == 0)
    {
		result = send_shutdown_command();		//发送管理端口
	}
    else
    {
        fprintf(stderr, "Invalid command or missing arguments\n");
        result = -1;
    }

    close(sock);
    return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
