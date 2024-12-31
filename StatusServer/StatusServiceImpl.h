#pragma once
//#include <grpcpp/grpcpp.h>
//#include "message.grpc.pb.h"
//#include <mutex>
//
//using grpc::Server;
//using grpc::ServerBuilder;
//using grpc::ServerContext;
//using grpc::Status;
//using message::GetChatServerReq;
//using message::GetChatServerRsp;
//using message::LoginReq;
//using message::LoginRsp;
//using message::StatusService;
//
//class  ChatServer {
//public:
//	ChatServer():host(""),port(""),name(""),con_count(0){}
//	ChatServer(const ChatServer& cs):host(cs.host), port(cs.port), name(cs.name), con_count(cs.con_count){}
//	ChatServer& operator=(const ChatServer& cs) {
//		if (&cs == this) {
//			return *this;
//		}
//
//		host = cs.host;
//		name = cs.name;
//		port = cs.port;
//		con_count = cs.con_count;
//		return *this;
//	}
//	std::string host;
//	std::string port;
//	std::string name;
//	int con_count;
//};
//
////	StatusServer服务端，执行客户端要求的操作
//class StatusServiceImpl final : public StatusService::Service
//{
//public:
//	StatusServiceImpl();
//	Status GetChatServer(ServerContext* context, const GetChatServerReq* request,
//		GetChatServerRsp* reply) override;
//	Status Login(ServerContext* context, const LoginReq* request,
//		LoginRsp* reply) override;
//private:
//	void insertToken(int uid, std::string token);
//	ChatServer getChatServer();
//	std::unordered_map<std::string, ChatServer> _servers;	// 储存chatServer服务器的map
//	std::mutex _server_mtx;
//
//	std::unordered_map<int, std::string> _tokens;	// 绑定用户uid及其token
//	std::mutex _token_mtx;
//};

#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "message.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::StatusService;
using message::LoginReq;
using message::LoginRsp;

struct ChatServer {
    ChatServer() :host(""), port(""), name(""), con_count(0) {}
    std::string host;
    std::string port;
    std::string name;
    int con_count;
};

// StatusServer的grpc服务端类
class StatusServiceImpl final : public StatusService::Service
{
public:
    StatusServiceImpl();
    Status GetChatServer(ServerContext* context, const GetChatServerReq* request,   // 这个用于处理GateServer发过来的查询ChatServer信息的请求
        GetChatServerRsp* reply) override;
    Status Login(ServerContext* context, const LoginReq* request,                   // 这个用于处理ChatServer发过来的验证用户uid和token的请求
        LoginRsp* reply) override;                                                  // 没什么用了，ChatServer直接到redis中查询uid和token是否匹配

private:
    std::unordered_map<std::string, ChatServer> _servers;
    int _server_index;
    std::mutex _server_mtx; // 获取ChatServer信息时加的锁
    std::map<int, std::string> _tokens;  // 用于保存用户uid和对应token的信息，用于验证（弃用，token信息保存到redis中）
    std::mutex _token_mtx;  // 验证uid和token时加的锁（弃用）

    void insertToken(int uid, std::string token);   // 绑定用户uid和token
    ChatServer getChatServer();                     // 获取一个客户端连接数最少的的ChatServer信息
};

