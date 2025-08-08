# C/S 架构守护进程服务端与命令行客户端

## 系统概述
本系统包含一个守护进程服务端和一个命令行客户端：
- 服务端作为守护进程在后台运行
- 客户端通过命令行与服务端交互
- 每个客户端连接由服务端 fork 的子进程处理
- 服务端自动回收完成的子进程

## 编译与安装
```bash
# 编译所有组件
make

make -C common
make[1]: Entering directory '/home/ebmc/practice/common'
gcc -Wall -Wextra -pedantic -O2 -I../include -c -o utils.o utils.c
make[1]: Leaving directory '/home/ebmc/practice/common'
make -C server
make[1]: Entering directory '/home/ebmc/practice/server'
gcc -Wall -Wextra -pedantic -O2 -I../include -c -o server.o server.c
gcc -Wall -Wextra -pedantic -O2 -I../include -c -o request_handler.o request_handler.c
gcc -Wall -Wextra -pedantic -O2 -I../include  -o cs_daemon_server server.o request_handler.o ../common/utils.o
make[1]: Leaving directory '/home/ebmc/practice/server'
make -C client
make[1]: Entering directory '/home/ebmc/practice/client'
gcc -Wall -Wextra -pedantic -O2 -I../include -c -o client.o client.c
gcc -Wall -Wextra -pedantic -O2 -I../include  -o cs_daemon_client client.o ../common/utils.o
make[1]: Leaving directory '/home/ebmc/practice/client'
```

