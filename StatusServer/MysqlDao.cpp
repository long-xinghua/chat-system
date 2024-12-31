#include "MysqlDao.h"

MysqlPool::MysqlPool(const std::string& url, const std::string& user, const std::string& pass, const std::string& schema, int poolSize) 
	:url_(url), user_(user), pass_(pass), schema_(schema), poolSize_(poolSize) {
	try {
		for (int i = 0; i < poolSize_; i++) {
			sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();	// ��ȡmysql����ʵ����ͨ�������������������
			auto* con = driver->connect(url_, user_, pass_);
			con->setSchema(schema_);	// ��һ�����ݿ�����
			// ��ȡ��ǰʱ���
			auto currentTime = std::chrono::system_clock::now().time_since_epoch();
			long long timeStamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();	// ����λת��Ϊ��
			pool_.push(std::make_unique<SqlConnection>(con, timeStamp));	// make_unique���Զ��ڶ�������һ��SqlConnection���󲢷���unique_ptr������ָ��
		}

		_check_thread = std::thread([this]() {
			while (!b_stop_) {	// ���ӳػ�û�رվ�һֱѭ��
				checkConnection();
				std::this_thread::sleep_for(std::chrono::seconds(60));
			}
			});

		_check_thread.detach();	// ���̷߳��룬�ں�̨����
	}
	catch(sql::SQLException e){
		std::cout << "mysqlPool init failed, error is: " << e.what() << std::endl;
	}
}

MysqlPool::~MysqlPool() {
	std::unique_lock<std::mutex> lock(mutex_);
	//close();
	while(!pool_.empty()) {
		pool_.pop();
	}
}

void MysqlPool::checkConnection() {
	std::lock_guard<std::mutex> guard(mutex_);
	int poolSize = poolSize_;
	//��ȡ��ǰʱ���
	auto currentTime = std::chrono::system_clock::now().time_since_epoch();
	long long timeStamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();	// ����λת��Ϊ��
	for (int i = 0; i < poolSize; i++) {
		auto con = std::move(pool_.front());
		pool_.pop();
		// ����һ��Defer����ʵ������go�����е�defer���ܣ��˴���for������֮ǰһ����ִ��lambda���ʽ���ó���������con����pool_
		Defer defer([this, &con]() {
			pool_.push(std::move(con));
			});
		if (timeStamp - con->_last_oper_time < 300) {	// ��������ϴδӲ���ʱ��С��300��Ͳ����в���
			continue;
		}
		try {	// ̫��δ���в�����Ϊ����Ͽ����ӣ�������mysql��һ����ѯ����
			std::unique_ptr<sql::Statement> stmt(con->_con->createStatement());
			stmt->executeQuery("SELECT 1");
			con->_last_oper_time = timeStamp;
			std::cout << "execute 'keep alive' query" << std::endl;
		}
		catch(sql::SQLException e){// ��ѯʧ�ܣ����´���һ�������滻��������
			std::cout << "execute query failed, error is: " << e.what() << std::endl;
			sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();
			auto* newcon = driver->connect(url_, user_, pass_);
			newcon->setSchema(schema_);
			con->_con.reset(newcon);	// ��con�е�_con����ָ������Ϊnewcon
			con->_last_oper_time = timeStamp;
		}

	}
}

std::unique_ptr<SqlConnection> MysqlPool::getConnection() {
	std::unique_lock<std::mutex> lock(mutex_);
	cond_.wait(lock, [this]() {	// �����������еĺ�������false���ͷ����ȴ����ѣ�����true���������ִ��
		if (b_stop_) {
			return true;
		}
		return !pool_.empty();	// �������ѿ����ͷ����ȴ������̹߳黹����
		});
	if (b_stop_) {
		return nullptr;
	}
	std::unique_ptr<SqlConnection> con = std::move(pool_.front());
	pool_.pop();
	return con;	// ���ڷ�������Ϊunique_ptr<SqlConnection>��������ⲿ��auto a=getConnection()�Ļ���ʹ��unique_ptr���ƶ����壬��con������Ȩת��a
}

void MysqlPool::returnConnection(std::unique_ptr<SqlConnection> con) {
	std::unique_lock<std::mutex> lock(mutex_);
	if (b_stop_) {
		return;	// ���ӹر���Ҳû�й黹�ı�Ҫ��
	}
	pool_.push(std::move(con));
	cond_.notify_one();	// ����һ�����ڵȴ����߳���������
}

void MysqlPool::close() {
	b_stop_ = true;
	cond_.notify_all();
}

MysqlDao::MysqlDao() {
	auto& cfg = ConfigMgr::getInst();
	const auto& host = cfg["Mysql"]["Host"];
	const auto& port = cfg["Mysql"]["Port"];
	const auto& pwd = cfg["Mysql"]["Passwd"];
	const auto& schema = cfg["Mysql"]["Schema"];
	const auto& user = cfg["Mysql"]["User"];
	pool_.reset(new MysqlPool(host + ":" + port, user, pwd, schema, 5));
}
MysqlDao::~MysqlDao() {
	pool_->close();
}
int MysqlDao::regUser(const std::string& name, const std::string& email, const std::string& passwd) {
	auto con = pool_->getConnection();
	try {
		if (con == nullptr) {
			return -1;
		}
		// ׼�����ô洢���̣���mysql��ĺ�����
		std::unique_ptr<sql::PreparedStatement> stmt(con->_con->prepareStatement("CALL reg_user(?,?,?,@result)"));
		// ���ô������������
		stmt->setString(1, name);
		stmt->setString(2, email);
		stmt->setString(3, passwd);
		// ����PreparedStatement��ֱ��֧��ע�����������������Ҫʹ�ûỰ������������������ȡ���������ֵ

		// ִ�д洢����
		stmt->execute();
		// ����洢���������˻Ự��������������ʽ��ȡ���������ֵ�������������ִ��SELECT��ѯ����ȡ����
	    // ���磬����洢����������һ���Ự����@result���洢������������������ȡ��
		std::unique_ptr<sql::Statement> stmtResult(con->_con->createStatement());
		std::unique_ptr<sql::ResultSet> res(stmtResult->executeQuery("SELECT @result AS result"));	// ��ѯresult��ֵ��ȡ������
		if (res->next()) {
			int result = res->getInt("result");	// resultΪint���ͣ�������getInt
			std::cout << "mysql query result: " << result << std::endl;
			pool_->returnConnection(std::move(con));
			return result;
		}
		pool_->returnConnection(std::move(con));
		return -1;	// -1��ʾ����ʧ��
	}
	catch (sql::SQLException& e) {
		pool_->returnConnection(std::move(con));
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return -1;
	}
	
}

bool MysqlDao::checkEmail(const std::string& name, const std::string& email) {
	auto con = pool_->getConnection();
	try {
		if (con == nullptr) {
			std::cout << "cannot get a Mysql connection" << std::endl;
			return false;
		}
		// ׼����ѯ���
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("SELECT name FROM user WHERE email = ?"));
		// ���ô���Ĳ���
		// std::string quotedEmail = "'" + email + "'";
		pstmt->setString(1, email);
		// ִ�в�ѯ
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

		while (res->next()) {
			std::cout << "����while (res->next())�����" << std::endl;
			std::string nameColumn = "name";
			// ��res->getString()�в���ֱ��ʹ��name����Ϊ������name�Ǹ����������û�����������"name"�ַ�������������
			std::cout << "check user: " << res->getString(nameColumn) << std::endl;
			if (res->getString(nameColumn) != name) {	// �û��������䲻ƥ��
				std::cout << "name in mysql: " << res->getString(nameColumn) << ", input name: " << name << std::endl;
				pool_->returnConnection(std::move(con));
				return false;
			}
			pool_->returnConnection(std::move(con));
			return true;
		}
		std::cout << "ִ�е���" << std::endl;
		// ���res->next()Ϊ��˵��mysql���Ҳ���������䣬Ҳ���û������ڵ����
		pool_->returnConnection(std::move(con));
		return false;
		//return true;
	}
	catch (sql::SQLException& e) {
		pool_->returnConnection(std::move(con));
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}

bool MysqlDao::updatePasswd(const std::string& email, const std::string& passwd) {
	auto con = pool_->getConnection();
	try {
		if (con == nullptr) {
			std::cout << "cannot get a Mysql connection" << std::endl;
			return false;
		}
		// ׼����ѯ���(������䲻���ڷ��ص��ǿյ����ݣ���ƥ��ʱҲ���nameƥ�䲻��)
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("UPDATE user SET passwd = ? WHERE email = ?"));
		// ���ô���Ĳ���
		pstmt->setString(1, passwd);
		pstmt->setString(2, email);
		// ִ�и���
		int updateCount = pstmt->executeUpdate();

		std::cout << "Updated rows: " << updateCount << std::endl;

		pool_->returnConnection(std::move(con));
		return true;
	}
	catch (sql::SQLException& e) {
		pool_->returnConnection(std::move(con));
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}

bool MysqlDao::checkPasswd(const std::string& email, const std::string& passwd, UserInfo& userInfo) {
	auto con = pool_->getConnection();

	if (con == nullptr) {
		std::cout << "cannot get a Mysql connection" << std::endl;
		return false;
	}

	Defer defer([this, &con]() {
		pool_->returnConnection(std::move(con));
		});

	try {	
		// ׼����ѯ���
		std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("SELECT * FROM user WHERE email = ?"));
		// ���ô���Ĳ���
		pstmt->setString(1, email);
		// ִ�в�ѯ
		std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
		std::string truePasswd = "";
		while (res->next()) {
			truePasswd = res->getString("passwd");
			//�����ѯ��������
			std::cout << "get password from mysql: " << truePasswd << std::endl;
			// pool_->returnConnection(std::move(con));
			break;
		}
		// ���벻ƥ�䣨����ѯ���䲻���ڵ�ʱ��truePasswd����Ĭ�ϵĿ��ַ���Ҳ����ƥ���ϣ�
		if (passwd != truePasswd) {	
			return false;
		}
		userInfo.name = res->getString("name");
		userInfo.email = res->getString("email");
		userInfo.passwd = truePasswd;
		userInfo.uid = res->getInt("uid");
		return true;
	}
	catch (sql::SQLException& e) {
		// pool_->returnConnection(std::move(con));
		std::cerr << "SQLException: " << e.what();
		std::cerr << " (MySQL error code: " << e.getErrorCode();
		std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		return false;
	}
}
