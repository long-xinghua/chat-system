#include "MysqlMgr.h"

MysqlMgr::~MysqlMgr() {

}

int MysqlMgr::regUser(const std::string& name, const std::string& email, const std::string& pwd)
{
    return _dao.regUser(name, email, pwd);
}

bool MysqlMgr::checkEmail(const std::string& name, const std::string& email) {
    return _dao.checkEmail(name, email);
}
bool MysqlMgr::updatePasswd(const std::string& email, const std::string& passwd) {
    return _dao.updatePasswd(email, passwd);
}

bool MysqlMgr::checkPasswd(const std::string& email, const std::string& passwd, UserInfo& userInfo) {
    return _dao.checkPasswd(email, passwd,  userInfo);
}

MysqlMgr::MysqlMgr() {
}