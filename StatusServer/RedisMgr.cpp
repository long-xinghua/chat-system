#include "RedisMgr.h"

RedisMgr::RedisMgr() {
    auto& gCfgMgr = ConfigMgr::getInst();
    std::string host = gCfgMgr["Redis"]["Host"];
    std::string port = gCfgMgr["Redis"]["Port"];
    std::string passwd = gCfgMgr["Redis"]["Passwd"];
    // 创建一个连接池对象
    pool_.reset(new RedisConPool(5, host.c_str(), atoi(port.c_str()), passwd.c_str())); // 注意参数port的类型为int，不是char*
}

RedisMgr::~RedisMgr() {
    close();
}

bool RedisMgr::get(const std::string& key, std::string& value) {
    redisContext* connect = pool_->getConnection();
    
    if (connect == nullptr) {    // 获取连接失败
        return false;
    }
    // 只要取出连接了就要记得归还，否则会出现连接池空了的情况
    Defer defer([this, &connect]() {
        pool_->returnConnection(connect);
        });

    auto reply = (redisReply*)redisCommand(connect, "GET %s", key.c_str());    //redisCommand返回的是void*，给它转成redisReply*的类型
    if (reply == NULL) {
        std::cout << "[ GET  " << key << " ] failed" << std::endl;
        freeReplyObject(reply);  //reply不要用了的时候一定要用freeReplyObject将其释放掉
        return false;
    }

    if (reply->type != REDIS_REPLY_STRING) { // 正常的回复必须是string类型（redis的基本类型），判断一下
        std::cout << "[ GET  " << key << " ] failed, reply is not a string type, reply type: "<<reply->type << std::endl;
        freeReplyObject(reply);
        return false;
    }

    value = reply->str;  // 把获取的值给到value
    freeReplyObject(reply);

    std::cout << "Succeed to execute command [ GET " << key << "  ]" << std::endl;
    return true;
}

bool RedisMgr::set(const std::string& key, const std::string& value) {
    redisContext* connect = pool_->getConnection();
    if (connect == nullptr) {    // 获取连接失败
        return false;
    }
    // 只要取出连接了就要记得归还，否则会出现连接池空了的情况
    Defer defer([this, &connect]() {
        pool_->returnConnection(connect);
        });
    //执行redis命令行
    auto reply = (redisReply*)redisCommand(connect, "SET %s %s", key.c_str(), value.c_str());

    //如果返回NULL则说明执行失败
    if (NULL == reply)
    {
        std::cout << "Execut command [ SET " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }

    //如果执行失败则释放连接
    if (!(reply->type == REDIS_REPLY_STATUS && (strcmp(reply->str, "OK") == 0 || strcmp(reply->str, "ok") == 0)))  // 返回的字符串结果不为ok
    {
        std::cout << "Execut command [ SET " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }

    //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
    freeReplyObject(reply);
    std::cout << "Execut command [ SET " << key << "  " << value << " ] success ! " << std::endl;
    return true;
}

//bool RedisMgr::auth(const std::string& password)
//{
//    redisContext* connect = pool_->getConnection();
//    if (connect == nullptr) {    // 获取连接失败
//        return false;
//    }
//    auto reply = (redisReply*)redisCommand(connect, "AUTH %s", password.c_str());
//    if (reply->type == REDIS_REPLY_ERROR) {
//        std::cout << "认证失败" << std::endl;
//        //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
//        freeReplyObject(reply);
//        return false;
//    }
//    else {
//        //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
//        freeReplyObject(reply);
//        std::cout << "认证成功" << std::endl;
//        return true;
//    }
//}

bool RedisMgr::lPush(const std::string& key, const std::string& value)
{
    redisContext* connect = pool_->getConnection();
    if (connect == nullptr) {    // 获取连接失败
        return false;
    }
    // 只要取出连接了就要记得归还，否则会出现连接池空了的情况
    Defer defer([this, &connect]() {
        pool_->returnConnection(connect);
        });
    auto reply = (redisReply*)redisCommand(connect, "LPUSH %s %s", key.c_str(), value.c_str());
    if (NULL == reply)
    {
        std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }

    if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {  // 操作后会返回列表的长度
        std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }

    std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] success ! " << std::endl;
    freeReplyObject(reply);
    return true;
}

bool RedisMgr::lPop(const std::string& key, std::string& value) {
    redisContext* connect = pool_->getConnection();
    if (connect == nullptr) {    // 获取连接失败
        return false;
    }
    // 只要取出连接了就要记得归还，否则会出现连接池空了的情况
    Defer defer([this, &connect]() {
        pool_->returnConnection(connect);
        });
    auto reply = (redisReply*)redisCommand(connect, "LPOP %s ", key.c_str());
    if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
        std::cout << "Execut command [ LPOP " << key << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    value = reply->str;    // 该值为弹出的元素值
    std::cout << "Execut command [ LPOP " << key << " ] success ! " << std::endl;
    freeReplyObject(reply);
    return true;
}

bool RedisMgr::rPush(const std::string& key, const std::string& value) {
    redisContext* connect = pool_->getConnection();
    if (connect == nullptr) {    // 获取连接失败
        return false;
    }
    // 只要取出连接了就要记得归还，否则会出现连接池空了的情况
    Defer defer([this, &connect]() {
        pool_->returnConnection(connect);
        });
    auto reply = (redisReply*)redisCommand(connect, "RPUSH %s %s", key.c_str(), value.c_str());
    if (NULL == reply)
    {
        std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }

    if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
        std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }

    std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] success ! " << std::endl;
    freeReplyObject(reply);
    return true;
}

bool RedisMgr::rPop(const std::string& key, std::string& value) {
    redisContext* connect = pool_->getConnection();
    if (connect == nullptr) {    // 获取连接失败
        return false;
    }
    // 只要取出连接了就要记得归还，否则会出现连接池空了的情况
    Defer defer([this, &connect]() {
        pool_->returnConnection(connect);
        });
    auto reply = (redisReply*)redisCommand(connect, "RPOP %s ", key.c_str());
    if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
        std::cout << "Execut command [ RPOP " << key << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    value = reply->str;
    std::cout << "Execut command [ RPOP " << key << " ] success ! " << std::endl;
    freeReplyObject(reply);
    return true;
}

bool RedisMgr::hSet(const std::string& key, const std::string& hkey, const std::string& value) {
    redisContext* connect = pool_->getConnection();
    if (connect == nullptr) {    // 获取连接失败
        return false;
    }
    // 只要取出连接了就要记得归还，否则会出现连接池空了的情况
    Defer defer([this, &connect]() {
        pool_->returnConnection(connect);
        });
    auto reply = (redisReply*)redisCommand(connect, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) { // 如果是新字段插入返回1，如果字段已存在并且其值被更新，返回0
        std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << value << " ] success ! " << std::endl;
    freeReplyObject(reply);
    return true;
}


bool RedisMgr::hSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen)
{
    redisContext* connect = pool_->getConnection();
    if (connect == nullptr) {    // 获取连接失败
        return false;
    }
    // 只要取出连接了就要记得归还，否则会出现连接池空了的情况
    Defer defer([this, &connect]() {
        pool_->returnConnection(connect);
        });
    const char* argv[4];
    size_t argvlen[4];
    argv[0] = "HSET";
    argvlen[0] = 4;
    argv[1] = key;
    argvlen[1] = strlen(key);
    argv[2] = hkey;
    argvlen[2] = strlen(hkey);
    argv[3] = hvalue;
    argvlen[3] = hvaluelen;
    
    auto reply = (redisReply*)redisCommandArgv(connect, 4, argv, argvlen);
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
        std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << hvalue << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << hvalue << " ] success ! " << std::endl;
    freeReplyObject(reply);
    return true;
}

std::string RedisMgr::hGet(const std::string& key, const std::string& hkey)
{
    redisContext* connect = pool_->getConnection();
    if (connect == nullptr) {    // 获取连接失败
        return "";
    }
    // 只要取出连接了就要记得归还，否则会出现连接池空了的情况
    Defer defer([this, &connect]() {
        pool_->returnConnection(connect);
        });
    const char* argv[3];
    size_t argvlen[3];
    argv[0] = "HGET";
    argvlen[0] = 4;
    argv[1] = key.c_str();
    argvlen[1] = key.length();
    argv[2] = hkey.c_str();
    argvlen[2] = hkey.length();
    
    auto reply = (redisReply*)redisCommandArgv(connect, 3, argv, argvlen);
    if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
        freeReplyObject(reply);
        std::cout << "Execut command [ HGet " << key << " " << hkey << "  ] failure ! " << std::endl;
        return "";
    }

    std::string value = reply->str;
    freeReplyObject(reply);
    std::cout << "Execut command [ HGet " << key << " " << hkey << " ] success ! " << std::endl;
    return value;
}

bool RedisMgr::del(const std::string& key)
{
    redisContext* connect = pool_->getConnection();
    if (connect == nullptr) {    // 获取连接失败
        return false;
    }
    // 只要取出连接了就要记得归还，否则会出现连接池空了的情况
    Defer defer([this, &connect]() {
        pool_->returnConnection(connect);
        });
    auto reply = (redisReply*)redisCommand(connect, "DEL %s", key.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) { // 返回被删除键的数量，键不存在返回0，删了一个就返回1...
        std::cout << "Execut command [ Del " << key << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    std::cout << "Execut command [ Del " << key << " ] success ! " << std::endl;
    freeReplyObject(reply);
    return true;
}

bool RedisMgr::hDel(const std::string& key, const std::string& field) {
    auto connect = pool_->getConnection();
    if (connect == nullptr) {
        return false;
    }

    Defer defer([&connect, this]() {
        pool_->returnConnection(connect);
        });

    redisReply* reply = (redisReply*)redisCommand(connect, "HDEL %s %s", key.c_str(), field.c_str());
    if (reply == nullptr) {
        std::cerr << "HDEL command failed" << std::endl;
        return false;
    }

    bool success = false;
    if (reply->type == REDIS_REPLY_INTEGER) {
        success = reply->integer > 0;   // 删除数量应该大于0
    }

    freeReplyObject(reply);
    return success;
}

bool RedisMgr::existsKey(const std::string& key)
{
    redisContext* connect = pool_->getConnection();
    if (connect == nullptr) {    // 获取连接失败
        return false;
    }
    // 只要取出连接了就要记得归还，否则会出现连接池空了的情况
    Defer defer([this, &connect]() {
        pool_->returnConnection(connect);
        });
    auto reply = (redisReply*)redisCommand(connect, "exists %s", key.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER || reply->integer == 0) {   // 返回值为0代表不存在，为1代表存在
        std::cout << "Not Found [ Key " << key << " ]  ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    std::cout << " Found [ Key " << key << " ] exists ! " << std::endl;
    freeReplyObject(reply);
    return true;
}

void RedisMgr::close()
{
    // redisFree(_connect);
    pool_->close();
}
