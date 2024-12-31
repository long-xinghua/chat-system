#pragma once
#include "const.h"

class RedisConPool {    // redis连接池形式与grpc连接池类似
public:
    RedisConPool(size_t poolSize, const char* host, int port, const char* pwd); // 指定服务端ip和端口，创建poolSize个连接
    ~RedisConPool();

    redisContext* getConnection();
    void returnConnection(redisContext* context);
    void close();

private:
    std::atomic<bool> b_stop_;
    size_t poolSize_;
    const char* host_;
    int port_;
    std::queue<redisContext*> connections_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

