#pragma once
#include <vector>
#include <boost/asio.hpp>
#include "Singleton.h"
class AsioIOServicePool:public Singleton<AsioIOServicePool>	// io_context早期时名字叫IOService
{
	friend Singleton<AsioIOServicePool>;
public:
	using IOService = boost::asio::io_context;	// 给io_context取个老的IOService的名字
	using Work = boost::asio::io_context::work;	// io_context调用io_context.run()时如果没有绑定任何事件会直接返回，而如果绑定了一个Work就不会退出
	using WorkPtr = std::unique_ptr<Work>;	// 给work包装到unique_ptr中
	~AsioIOServicePool();
	AsioIOServicePool(const AsioIOServicePool&) = delete;
	AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;
	// 使用 round-robin 的方式返回一个 io_service
	boost::asio::io_context& GetIOService();	// 返回一个上下文
	void Stop();
private:
	AsioIOServicePool(std::size_t size = 2/*std::thread::hardware_concurrency()*/);	// 单例类构造函数放私有里,可以用std::thread::hardware_concurrency()根据cpu核心数创建线程
	std::vector<IOService> _ioServices;	//	存放上下文
	std::vector<WorkPtr> _works;	//有多少IOService就有多少WorkPtr，防止上下文未绑定事件时直接退出
	std::vector<std::thread> _threads;	// 有多少上下文就有多少线程，每个上下文都跑在单独的线程中
	std::size_t  _nextIOService;	// 下一个IOService的索引
};

