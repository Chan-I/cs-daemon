# C/S 架构守护进程服务端与命令行客户端小项目
可以用作cs模型socket通信，同时集成了一个Flex和bison的小功能。
## 系统概述
本系统包含一个守护进程服务端和一个命令行客户端：
- 服务端作为守护进程在后台运行
- 客户端通过命令行与服务端交互
- 每个客户端连接由服务端 fork 的子进程处理
- 服务端自动回收完成的子进程
- 服务端能够做一些最简单的词法语法分析。

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

## 使用
server:
```bash
$ ./server/cs_daemon_server
```
client:
```bash
$ ./client/cs_daemon_client info
Server Info: Version 1.0, pid:3995
$ ./client/cs_daemon_client exec df -Th
文件系统       类型   大小  已用  可用 已用% 挂载点
tmpfs          tmpfs  790M  1.8M  788M    1% /run
/dev/sda2      ext4    59G   11G   46G   19% /
tmpfs          tmpfs  3.9G     0  3.9G    0% /dev/shm
tmpfs          tmpfs  5.0M  8.0K  5.0M    1% /run/lock
tmpfs          tmpfs  790M  112K  790M    1% /run/user/1000
$ ./client/cs_daemon_client calc '(add (1 2 3))'
calc parse success
$ ./client/cs_daemon_client calc '(add (1 2 3)))))'
calc parse failed!

#关闭server服务，执行5秒后ps -ef | grep cs_daemon_server将查看不到进程信息
$ ./client/cs_daemon_client stop
Stopping server  
```
