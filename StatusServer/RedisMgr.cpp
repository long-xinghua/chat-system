#include "RedisMgr.h"

RedisMgr::RedisMgr() {
    auto& gCfgMgr = ConfigMgr::getInst();
    std::string host = gCfgMgr["Redis"]["Host"];
    std::string port = gCfgMgr["Redis"]["Port"];
    std::string passwd = gCfgMgr["Redis"]["Passwd"];
    // ����һ�����ӳض���
    pool_.reset(new RedisConPool(5, host.c_str(), atoi(port.c_str()), passwd.c_str())); // ע�����port������Ϊint������char*
}

RedisMgr::~RedisMgr() {
    close();
}

bool RedisMgr::get(const std::string& key, std::string& value) {
    redisContext* connect = pool_->getConnection();
    
    if (connect == nullptr) {    // ��ȡ����ʧ��
        return false;
    }
    // ֻҪȡ�������˾�Ҫ�ǵù黹�������������ӳؿ��˵����
    Defer defer([this, &connect]() {
        pool_->returnConnection(connect);
        });

    auto reply = (redisReply*)redisCommand(connect, "GET %s", key.c_str());    //redisCommand���ص���void*������ת��redisReply*������
    if (reply == NULL) {
        std::cout << "[ GET  " << key << " ] failed" << std::endl;
        freeReplyObject(reply);  //reply��Ҫ���˵�ʱ��һ��Ҫ��freeReplyObject�����ͷŵ�
        return false;
    }

    if (reply->type != REDIS_REPLY_STRING) { // �����Ļظ�������string���ͣ�redis�Ļ������ͣ����ж�һ��
        std::cout << "[ GET  " << key << " ] failed, reply is not a string type, reply type: "<<reply->type << std::endl;
        freeReplyObject(reply);
        return false;
    }

    value = reply->str;  // �ѻ�ȡ��ֵ����value
    freeReplyObject(reply);

    std::cout << "Succeed to execute command [ GET " << key << "  ]" << std::endl;
    return true;
}

bool RedisMgr::set(const std::string& key, const std::string& value) {
    redisContext* connect = pool_->getConnection();
    if (connect == nullptr) {    // ��ȡ����ʧ��
        return false;
    }
    // ֻҪȡ�������˾�Ҫ�ǵù黹�������������ӳؿ��˵����
    Defer defer([this, &connect]() {
        pool_->returnConnection(connect);
        });
    //ִ��redis������
    auto reply = (redisReply*)redisCommand(connect, "SET %s %s", key.c_str(), value.c_str());

    //�������NULL��˵��ִ��ʧ��
    if (NULL == reply)
    {
        std::cout << "Execut command [ SET " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }

    //���ִ��ʧ�����ͷ�����
    if (!(reply->type == REDIS_REPLY_STATUS && (strcmp(reply->str, "OK") == 0 || strcmp(reply->str, "ok") == 0)))  // ���ص��ַ��������Ϊok
    {
        std::cout << "Execut command [ SET " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }

    //ִ�гɹ� �ͷ�redisCommandִ�к󷵻ص�redisReply��ռ�õ��ڴ�
    freeReplyObject(reply);
    std::cout << "Execut command [ SET " << key << "  " << value << " ] success ! " << std::endl;
    return true;
}

//bool RedisMgr::auth(const std::string& password)
//{
//    redisContext* connect = pool_->getConnection();
//    if (connect == nullptr) {    // ��ȡ����ʧ��
//        return false;
//    }
//    auto reply = (redisReply*)redisCommand(connect, "AUTH %s", password.c_str());
//    if (reply->type == REDIS_REPLY_ERROR) {
//        std::cout << "��֤ʧ��" << std::endl;
//        //ִ�гɹ� �ͷ�redisCommandִ�к󷵻ص�redisReply��ռ�õ��ڴ�
//        freeReplyObject(reply);
//        return false;
//    }
//    else {
//        //ִ�гɹ� �ͷ�redisCommandִ�к󷵻ص�redisReply��ռ�õ��ڴ�
//        freeReplyObject(reply);
//        std::cout << "��֤�ɹ�" << std::endl;
//        return true;
//    }
//}

bool RedisMgr::lPush(const std::string& key, const std::string& value)
{
    redisContext* connect = pool_->getConnection();
    if (connect == nullptr) {    // ��ȡ����ʧ��
        return false;
    }
    // ֻҪȡ�������˾�Ҫ�ǵù黹�������������ӳؿ��˵����
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

    if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {  // ������᷵���б�ĳ���
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
    if (connect == nullptr) {    // ��ȡ����ʧ��
        return false;
    }
    // ֻҪȡ�������˾�Ҫ�ǵù黹�������������ӳؿ��˵����
    Defer defer([this, &connect]() {
        pool_->returnConnection(connect);
        });
    auto reply = (redisReply*)redisCommand(connect, "LPOP %s ", key.c_str());
    if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
        std::cout << "Execut command [ LPOP " << key << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    value = reply->str;    // ��ֵΪ������Ԫ��ֵ
    std::cout << "Execut command [ LPOP " << key << " ] success ! " << std::endl;
    freeReplyObject(reply);
    return true;
}

bool RedisMgr::rPush(const std::string& key, const std::string& value) {
    redisContext* connect = pool_->getConnection();
    if (connect == nullptr) {    // ��ȡ����ʧ��
        return false;
    }
    // ֻҪȡ�������˾�Ҫ�ǵù黹�������������ӳؿ��˵����
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
    if (connect == nullptr) {    // ��ȡ����ʧ��
        return false;
    }
    // ֻҪȡ�������˾�Ҫ�ǵù黹�������������ӳؿ��˵����
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
    if (connect == nullptr) {    // ��ȡ����ʧ��
        return false;
    }
    // ֻҪȡ�������˾�Ҫ�ǵù黹�������������ӳؿ��˵����
    Defer defer([this, &connect]() {
        pool_->returnConnection(connect);
        });
    auto reply = (redisReply*)redisCommand(connect, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) { // ��������ֶβ��뷵��1������ֶ��Ѵ��ڲ�����ֵ�����£�����0
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
    if (connect == nullptr) {    // ��ȡ����ʧ��
        return false;
    }
    // ֻҪȡ�������˾�Ҫ�ǵù黹�������������ӳؿ��˵����
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
    if (connect == nullptr) {    // ��ȡ����ʧ��
        return "";
    }
    // ֻҪȡ�������˾�Ҫ�ǵù黹�������������ӳؿ��˵����
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
    if (connect == nullptr) {    // ��ȡ����ʧ��
        return false;
    }
    // ֻҪȡ�������˾�Ҫ�ǵù黹�������������ӳؿ��˵����
    Defer defer([this, &connect]() {
        pool_->returnConnection(connect);
        });
    auto reply = (redisReply*)redisCommand(connect, "DEL %s", key.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) { // ���ر�ɾ�������������������ڷ���0��ɾ��һ���ͷ���1...
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
        success = reply->integer > 0;   // ɾ������Ӧ�ô���0
    }

    freeReplyObject(reply);
    return success;
}

bool RedisMgr::existsKey(const std::string& key)
{
    redisContext* connect = pool_->getConnection();
    if (connect == nullptr) {    // ��ȡ����ʧ��
        return false;
    }
    // ֻҪȡ�������˾�Ҫ�ǵù黹�������������ӳؿ��˵����
    Defer defer([this, &connect]() {
        pool_->returnConnection(connect);
        });
    auto reply = (redisReply*)redisCommand(connect, "exists %s", key.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER || reply->integer == 0) {   // ����ֵΪ0�������ڣ�Ϊ1�������
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
