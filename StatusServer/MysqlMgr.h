#pragma once
// MysqlMgrΪ�����࣬��������MysqlDao�ĸ���
#include "const.h"
#include "MysqlDao.h"

class MysqlMgr : public Singleton<MysqlMgr>
{
    friend class Singleton<MysqlMgr>;
public:
    ~MysqlMgr();
    int regUser(const std::string& name, const std::string& email, const std::string& pwd); // ע���û��������û�uid��uidΪ-1��ʾע��ʧ�ܣ�uidΪ0��ʾ�û��Ѵ���
    bool checkEmail(const std::string& name, const std::string& email);   // ������ݿ���user��email�Ƿ��Ӧ
    bool updatePasswd(const std::string& name, const std::string& email); // �����û�����
    bool checkPasswd(const std::string& email, const std::string& passwd, UserInfo& userInfo); // ��¼ʱ���email��passwd�Ƿ��Ӧ
private:
    MysqlMgr();
    MysqlDao  _dao;
};

