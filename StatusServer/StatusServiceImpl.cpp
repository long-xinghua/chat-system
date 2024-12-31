#include "StatusServiceImpl.h"
#include "ConfigMgr.h"
#include "const.h"
#include "RedisMgr.h"

std::string generate_unique_string() {
    // 创建UUID对象
    boost::uuids::uuid uuid = boost::uuids::random_generator()();

    // 将UUID转换为字符串
    std::string unique_string = to_string(uuid);

    return unique_string;
}

// 测试用
//Status StatusServiceImpl::GetChatServer(ServerContext* context, const GetChatServerReq* request, GetChatServerRsp* reply)
//{
//    std::string prefix("status server has received :  ");
//    _server_index = (_server_index++) % (_servers.size());
//    auto& server = _servers[_server_index];
//    reply->set_host(server.host);
//    reply->set_port(server.port);
//    reply->set_error(ErrorCodes::Success);
//    reply->set_token(generate_unique_string());
//    return Status::OK;
//}

Status StatusServiceImpl::GetChatServer(ServerContext* context, const GetChatServerReq* request, GetChatServerRsp* reply)
{
    std::string prefix("status server has received :  ");
    std::cout << "StatusServer has received: uid:" << request->uid()<<std::endl;
    const auto& server = getChatServer();
    reply->set_host(server.host);
    reply->set_port(server.port);
    reply->set_error(ErrorCodes::Success);
    reply->set_token(generate_unique_string());
    std::cout << "reply->token():" << reply->token() << std::endl;
   insertToken(request->uid(), reply->token());   // 绑定用户uid和token
    return Status::OK;
}

Status StatusServiceImpl::Login(ServerContext* context, const LoginReq* request, LoginRsp* reply) {
    auto uid = request->uid();
	auto token = request->token();

	std::string uid_str = std::to_string(uid);
	std::string token_key = USERTOKENPREFIX + uid_str;
	std::string token_value = "";
	bool success = RedisMgr::getInstance()->get(token_key, token_value);
	if (!success) {
		reply->set_error(ErrorCodes::UidInvalid);
		return Status::OK;
	}
	
	if (token_value != token) {
		reply->set_error(ErrorCodes::TokenInvalid);
		return Status::OK;
	}
	reply->set_error(ErrorCodes::Success);
	reply->set_uid(uid);
	reply->set_token(token);
	return Status::OK;
}


ChatServer StatusServiceImpl::getChatServer() {
    std::lock_guard<std::mutex> guard(_server_mtx);
    
    ChatServer minServer;
    if (_servers.empty()) {
        std::cout << "No available ChatServer!" << std::endl;
        return minServer;
    }
    minServer.con_count = INT_MAX;
    // 寻找连接数最少的server
    for (auto& server : _servers) {
        auto count = RedisMgr::getInstance()->hGet(LOGIN_COUNT, server.second.name);
        if (count.empty()) {
            server.second.con_count = INT_MAX;  // 说明该ChatServer还没打开
        }
        else {
            server.second.con_count = stoi(count);
        }

        if (server.second.con_count < minServer.con_count) {
            minServer = server.second;
        }
    }
    std::cout << "find minServer: " << minServer.name << ", connection count: " << minServer.con_count << std::endl;
    return minServer;

}


void StatusServiceImpl::insertToken(int uid, std::string token)
{
    std::string uid_str = std::to_string(uid);
    std::string token_key = USERTOKENPREFIX + uid_str;
    RedisMgr::getInstance()->set(token_key, token);
}

StatusServiceImpl::StatusServiceImpl() :_server_index(0)
{
    auto& cfg = ConfigMgr::getInst();
	auto server_list = cfg["ChatServers"]["Name"];

	std::stringstream ss(server_list);
    std::vector<std::string> words;
	std::string word;

	while (std::getline(ss, word, ',')) {
		words.push_back(word);
	}

	for (auto& word : words) {
		if (cfg[word]["Name"].empty()) {    // 找不到这个服务器就跳过
			continue;
		}
		ChatServer server;
		server.port = cfg[word]["Port"];
		server.host = cfg[word]["Host"];
		server.name = cfg[word]["Name"];
		_servers[server.name] = server;
	}
}

