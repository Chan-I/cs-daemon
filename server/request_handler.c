#include "server/request_handler.h"
#include "pse/module.h"

volatile sig_atomic_t child_shutdown = 0;


// 处理客户端请求
int handle_client(int sock)
{
    RequestHeader req_header;
    ResponseHeader resp_header;
    char buffer[MAX_DATA_LEN];


    // 读取请求头
    ssize_t bytes_read = read(sock, &req_header, sizeof(RequestHeader));
    if (bytes_read != sizeof(RequestHeader))
    {
        syslog(LOG_INFO, "Invalid request header");
        return -1;
    }

    // 验证协议版本
    if (req_header.version != 1)
    {
        syslog(LOG_ERR, "Unsupported protocol version: %d", req_header.version);
        resp_header.version = 1;
        resp_header.status = 0x01; // 不支持的版本
        resp_header.data_len = 0;
        if (write(sock, &resp_header, sizeof(ResponseHeader)) == -1)
			syslog(LOG_ERR, "write ResponseHeader error");
        return -1;
    }

    // 读取请求数据
    if (req_header.data_len > 0)
    {
        if (req_header.data_len > MAX_DATA_LEN)
        {
            syslog(LOG_ERR, "Request data too large: %d bytes", req_header.data_len);
            resp_header.status = 0x02; // 数据过大
            resp_header.data_len = 0;
            if (write(sock, &resp_header, sizeof(ResponseHeader)) == -1)
				syslog(LOG_ERR, "write ResponseHeader error");
            return -1;
        }

        bytes_read = read(sock, buffer, req_header.data_len);
        if (bytes_read != req_header.data_len)
        {
            syslog(LOG_ERR, "Incomplete request data");
            return -1;
        }
        buffer[req_header.data_len] = '\0'; // 确保字符串终止
    }

    // 准备响应头
    resp_header.version = 1;
    resp_header.status = 0x00; // 成功
    resp_header.data_len = 0;

    // 处理不同命令
    switch (req_header.cmd)
    {
        case CMD_ECHO:
            syslog(LOG_INFO, "ECHO command received");
            resp_header.data_len = req_header.data_len;
            if (write(sock, &resp_header, sizeof(ResponseHeader)) == -1)
            {
				syslog(LOG_ERR, "write ResponseHeader error");
				return -1;
            }
            if (write(sock, buffer, req_header.data_len) == -1)
            {
				syslog(LOG_ERR, "write resp_header error");
				return -1;
            }
            break;

        case CMD_GET_INFO:
            syslog(LOG_INFO, "GET_INFO command received");
            char info[128] = {0};
            sprintf(info, "Server Info: Version 1.0, pid:%d\n", getpid());
            resp_header.data_len = strlen(info);
            if (write(sock, &resp_header, sizeof(ResponseHeader)) == -1)
			{
				syslog(LOG_ERR, "write ResponseHeader error");
				return -1;
            }
            if (write(sock, info, resp_header.data_len) == -1)
			{
				syslog(LOG_ERR, "write resp_header error");
				return -1;
            }
            break;

        case CMD_EXECUTE:
            char buffer_input[MAX_DATA_LEN + 5] = {0};
            syslog(LOG_INFO, "EXECUTE command received: %s", buffer);
            sprintf(buffer_input, "%s 2>&1", buffer);
            // 在实际应用中，这里应该进行严格的安全检查
			FILE* fp = popen(buffer_input, "r");
            if (fp == NULL)
            {
                resp_header.status = 0x03; // 执行失败
                if (write(sock, &resp_header, sizeof(ResponseHeader)) == -1)
                {
                	syslog(LOG_ERR, "write ResponseHeader error");
					return -1;
                }
                break;
            }

            // 读取命令输出
            char result[MAX_DATA_LEN] = {0};
            size_t len = fread(result, 1, MAX_DATA_LEN - 1, fp);
            pclose(fp);

            resp_header.data_len = len;
            if (write(sock, &resp_header, sizeof(ResponseHeader)) == -1)
			{
            	syslog(LOG_ERR, "write ResponseHeader error");
				return -1;
	        }
            if (write(sock, result, len) == -1)
            {
				syslog(LOG_ERR, "Get result error");
				return -1;
            }
            break;

        case CMD_CALC:
            syslog(LOG_INFO, "CALC command received");
            module *mod;
			mod = raw_parser(buffer);

			resp_header.data_len = 19;
			if (write(sock, &resp_header, sizeof(ResponseHeader)) == -1)
			{
            	syslog(LOG_ERR, "write ResponseHeader error");
				return -1;
	        }

			if (!mod->yyresult)
			{
				if (write(sock, "calc parse success\n", 19) == -1)
	            {
					syslog(LOG_ERR, "Get result error");
					return -1;
	            }
			}
			else
			{
				if (write(sock, "calc parse failed!\n", 19) == -1)
	            {
					syslog(LOG_ERR, "Get result error");
					return -1;
	            }
			}

            break;

        default:
            syslog(LOG_INFO, "Unknown command: 0x%02x", req_header.cmd);
            resp_header.status = 0x04; // 未知命令
            if (write(sock, &resp_header, sizeof(ResponseHeader)) == -1)
			{
            	syslog(LOG_ERR, "write ResponseHeader error");
				return -1;
            }
            break;
    }


	close(sock);

	return 0;
}
