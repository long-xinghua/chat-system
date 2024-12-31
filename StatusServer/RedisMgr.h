#pragma once
#include "const.h"
#include "RedisConPool.h"
#include "ConfigMgr.h"

class RedisMgr:public Singleton<RedisMgr>, public std::enable_shared_from_this<RedisMgr>	// redis�ĵ���������
{
    friend class Singleton<RedisMgr>;
public:
    ~RedisMgr();
    // bool connect(const std::string& host, int port);    // ���ӵ�redis������(�����ӳ������Ӻ��ˣ�����Ҫ�ú�����)
    bool get(const std::string& key, std::string& value);   // ͨ��key��ȡvalueֵ
    bool set(const std::string& key, const std::string& value); // ����key��ֵ
    // bool auth(const std::string& password); // �����������֤
    bool lPush(const std::string& key, const std::string& value);   // ��ֵ�����б����ࣨ������ֵ������ǰ���Ԫ��֮ǰ����keyΪҪ�������б�ļ���
    bool lPop(const std::string& key, std::string& value);  // �����б�����ߵ�ֵ�����ص�valueΪ���Ƴ���Ԫ��ֵ
    bool rPush(const std::string& key, const std::string& value);   //��ֵ�����б���Ҳ�
    bool rPop(const std::string& key, std::string& value);  // �����б����ұߵ�ֵ
    bool hSet(const std::string& key, const std::string& hkey, const std::string& value);   // ����ϣ���͵����ݽṹ�������ֶΣ�field����ֵ��value������ϣ���͵��������ж���ֶΣ��൱��һ��������key����ÿ���ֶ��ж�Ӧ��ֵ
    bool hSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen); // ���ذ汾���ʺ�ֵΪ���������ݶ�����string���ݵ����
    std::string hGet(const std::string& key, const std::string& hkey);  //�õ���ϣ�����������ֶΣ�������key����Ӧ��ֵ
    bool del(const std::string& key);   // ɾ��ָ���ļ�ֵ��
    bool hDel(const std::string& key, const std::string& field);        // ɾ����ϣ���������е�һ��ֵ
    bool existsKey(const std::string& key); // �жϼ��治����
    void close();   // �ͷ����ӣ���������д
private:
    RedisMgr();

    // redisContext* _connect; �������ӳ���߲�������
    // redisReply* _reply;
    std::unique_ptr<RedisConPool> pool_;
    
};

