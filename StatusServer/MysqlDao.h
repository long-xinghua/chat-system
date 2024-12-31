// Dao层为对数据库进行读写的层
#pragma once
#include "const.h"
#include "ConfigMgr.h"
#include <thread>
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/exception.h>

class SqlConnection {
public:
	SqlConnection(sql::Connection* con, int64_t last_time) :_con(con), _last_oper_time(last_time){}
	std::unique_ptr<sql::Connection> _con;
	int64_t _last_oper_time;	// 如果长时间不操作连接会断开，这个值用于记录最后一次执行操作时的时间戳
};

class MysqlPool {
public:
	MysqlPool(const std::string& url, const std::string& user, const std::string& pass, const std::string& schema, int poolSize);
	~MysqlPool();
	void checkConnection();	// 在后台运行，每隔一段时间检查池中的连接是否太久没进行操作，防止连接断开
	std::unique_ptr<SqlConnection> getConnection();	// 获取连接池中的一个连接
	void returnConnection(std::unique_ptr<SqlConnection> con);	// 归还连接
	void close();
private:
	std::string url_;	// 连接的url
	std::string user_;	// 连接的用户名
	std::string pass_;	// 密码
	std::string schema_;// 使用的数据库
	int poolSize_;
	std::queue<std::unique_ptr<SqlConnection>> pool_;
	std::mutex mutex_;
	std::condition_variable cond_;
	std::atomic<bool> b_stop_;
	std::thread _check_thread;	// 用于检测的线程，定时检测，发现连接超过一定时间没操作（如一分钟）就给mysql发请求，防止断开连接
};

// 在mysql中储存的用户信息
struct UserInfo {
	std::string name;
	std::string passwd;
	int uid;
	std::string email;
};

// 用于跟Mysql交互（Dao，Data Access Object）
class MysqlDao {
public:
	MysqlDao();
	~MysqlDao();
	int regUser(const std::string& name, const std::string& email, const std::string& passwd);
	bool checkEmail(const std::string& name, const std::string& email);
	//bool checkPasswd(const std::string name, const std::string passwd);
	bool updatePasswd(const std::string& name, const std::string& newPasswd);
	bool checkPasswd(const std::string& email, const std::string& passwd, UserInfo& userInfo);
private:
	std::unique_ptr<MysqlPool> pool_;
};

