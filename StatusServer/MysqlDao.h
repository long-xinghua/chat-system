// Dao��Ϊ�����ݿ���ж�д�Ĳ�
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
	int64_t _last_oper_time;	// �����ʱ�䲻�������ӻ�Ͽ������ֵ���ڼ�¼���һ��ִ�в���ʱ��ʱ���
};

class MysqlPool {
public:
	MysqlPool(const std::string& url, const std::string& user, const std::string& pass, const std::string& schema, int poolSize);
	~MysqlPool();
	void checkConnection();	// �ں�̨���У�ÿ��һ��ʱ������е������Ƿ�̫��û���в�������ֹ���ӶϿ�
	std::unique_ptr<SqlConnection> getConnection();	// ��ȡ���ӳ��е�һ������
	void returnConnection(std::unique_ptr<SqlConnection> con);	// �黹����
	void close();
private:
	std::string url_;	// ���ӵ�url
	std::string user_;	// ���ӵ��û���
	std::string pass_;	// ����
	std::string schema_;// ʹ�õ����ݿ�
	int poolSize_;
	std::queue<std::unique_ptr<SqlConnection>> pool_;
	std::mutex mutex_;
	std::condition_variable cond_;
	std::atomic<bool> b_stop_;
	std::thread _check_thread;	// ���ڼ����̣߳���ʱ��⣬�������ӳ���һ��ʱ��û��������һ���ӣ��͸�mysql�����󣬷�ֹ�Ͽ�����
};

// ��mysql�д�����û���Ϣ
struct UserInfo {
	std::string name;
	std::string passwd;
	int uid;
	std::string email;
};

// ���ڸ�Mysql������Dao��Data Access Object��
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

