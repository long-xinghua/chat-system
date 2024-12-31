#pragma once
// MysqlMgr为单例类，当作调用MysqlDao的父层
#include "const.h"
#include "MysqlDao.h"

class MysqlMgr : public Singleton<MysqlMgr>
{
    friend class Singleton<MysqlMgr>;
public:
    ~MysqlMgr();
    int regUser(const std::string& name, const std::string& email, const std::string& pwd); // 注册用户，返回用户uid，uid为-1表示注册失败，uid为0表示用户已存在
    bool checkEmail(const std::string& name, const std::string& email);   // 检查数据库中user和email是否对应
    bool updatePasswd(const std::string& name, const std::string& email); // 更新用户密码
    bool checkPasswd(const std::string& email, const std::string& passwd, UserInfo& userInfo); // 登录时检查email和passwd是否对应
private:
    MysqlMgr();
    MysqlDao  _dao;
};

