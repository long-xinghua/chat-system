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
////	StatusServer����ˣ�ִ�пͻ���Ҫ��Ĳ���
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
//	std::unordered_map<std::string, ChatServer> _servers;	// ����chatServer��������map
//	std::mutex _server_mtx;
//
//	std::unordered_map<int, std::string> _tokens;	// ���û�uid����token
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

// StatusServer��grpc�������
class StatusServiceImpl final : public StatusService::Service
{
public:
    StatusServiceImpl();
    Status GetChatServer(ServerContext* context, const GetChatServerReq* request,   // ������ڴ���GateServer�������Ĳ�ѯChatServer��Ϣ������
        GetChatServerRsp* reply) override;
    Status Login(ServerContext* context, const LoginReq* request,                   // ������ڴ���ChatServer����������֤�û�uid��token������
        LoginRsp* reply) override;                                                  // ûʲô���ˣ�ChatServerֱ�ӵ�redis�в�ѯuid��token�Ƿ�ƥ��

private:
    std::unordered_map<std::string, ChatServer> _servers;
    int _server_index;
    std::mutex _server_mtx; // ��ȡChatServer��Ϣʱ�ӵ���
    std::map<int, std::string> _tokens;  // ���ڱ����û�uid�Ͷ�Ӧtoken����Ϣ��������֤�����ã�token��Ϣ���浽redis�У�
    std::mutex _token_mtx;  // ��֤uid��tokenʱ�ӵ��������ã�

    void insertToken(int uid, std::string token);   // ���û�uid��token
    ChatServer getChatServer();                     // ��ȡһ���ͻ������������ٵĵ�ChatServer��Ϣ
};

