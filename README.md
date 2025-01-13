# chat-system

## 介绍  
C++聊天项目，涵盖grpc、并发线程、网络编程、数据库、Qt开发等多种技术的综合使用。  

## 架构设计
![项目架构如图所示](/assets/images/项目架构.png)

## 模块介绍  

### myChat  
Qt客户端，实现了注册、登录、重置密码、添加好友、查找用户、聊天等UI界面和相应的功能。  

### GateServer  
网关服务器，当客户端请求为登录、注册和重置密码时通过http协议访问该服务器。  

### VerifyServer  
验证服务器，当用户注册时GateServer向本服务器发送“发送验证码”请求，生成并向指定邮箱发送验证码，将验证码保存到Redis中并设置过期时间，验证用户邮箱。  

### StatusServer  
状态服务器，当用户登录时GateServer向本服务器发送“查询聊天服务器”请求，通过Redis查询现有的聊天服务器中负载最小的服务器，实现负载均衡，向GateServer返回服务器的ip地址。  

### ChatServers
分布式聊天服务器，为了承载更多客户端数量使用分布式设计，当用户请求为登录时使用tcp连接负载最小的服务器，并完成客户端之间的查找用户、添加好友和聊天等功能。  

## 使用到的C++库  
boost、jsoncpp、grpc、mysql-connector、hiredis。  

## 感谢B站up主恋恋风辰zack提供的开源项目  
[文档链接](https://gitbookcpp.llfc.club/sections/cpp/project/day01.html)