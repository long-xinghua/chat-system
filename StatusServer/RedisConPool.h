#pragma once
#include "const.h"

class RedisConPool {    // redis���ӳ���ʽ��grpc���ӳ�����
public:
    RedisConPool(size_t poolSize, const char* host, int port, const char* pwd); // ָ�������ip�Ͷ˿ڣ�����poolSize������
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

