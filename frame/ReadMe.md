# C 微服务框架

## 编译

```bash
# 编译所有
$ make 

# 清理所有
$ make clean
```

## 运行

```bash
# 启动服务端
$ ./server -c ./server.ini start

# 客户端调用服务端
$ ./client 127.0.0.1 
ok
Request times:1
Time taken for tests:0s 0ms
Requests per second:inf[#/sec]
Time per request:0.000000[ms]
Time per request:0.000000[ms](across all concurrent requests)
```
