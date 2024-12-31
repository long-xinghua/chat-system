#pragma once
#include "const.h"
#include "RedisConPool.h"
#include "ConfigMgr.h"

class RedisMgr:public Singleton<RedisMgr>, public std::enable_shared_from_this<RedisMgr>	// redis的单例管理类
{
    friend class Singleton<RedisMgr>;
public:
    ~RedisMgr();
    // bool connect(const std::string& host, int port);    // 连接到redis服务器(在连接池中连接好了，不需要该函数了)
    bool get(const std::string& key, std::string& value);   // 通过key获取value值
    bool set(const std::string& key, const std::string& value); // 设置key的值
    // bool auth(const std::string& password); // 用密码进行认证
    bool lPush(const std::string& key, const std::string& value);   // 将值插入列表的左侧（后插入的值会排在前面的元素之前），key为要操作的列表的键名
    bool lPop(const std::string& key, std::string& value);  // 弹出列表最左边的值，返回的value为被移除的元素值
    bool rPush(const std::string& key, const std::string& value);   //将值插入列表的右侧
    bool rPop(const std::string& key, std::string& value);  // 弹出列表最右边的值
    bool hSet(const std::string& key, const std::string& hkey, const std::string& value);   // 给哈希类型的数据结构中设置字段（field）和值（value），哈希类型的数据能有多个字段（相当于一个二级的key），每个字段有对应的值
    bool hSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen); // 重载版本，适合值为二进制数据而不是string数据的情况
    std::string hGet(const std::string& key, const std::string& hkey);  //得到哈希类型数据中字段（二级的key）对应的值
    bool del(const std::string& key);   // 删除指定的键值对
    bool hDel(const std::string& key, const std::string& field);        // 删除哈希类型数据中的一个值
    bool existsKey(const std::string& key); // 判断键存不存在
    void close();   // 释放连接，在析构中写
private:
    RedisMgr();

    // redisContext* _connect; 改用连接池提高并发能力
    // redisReply* _reply;
    std::unique_ptr<RedisConPool> pool_;
    
};

