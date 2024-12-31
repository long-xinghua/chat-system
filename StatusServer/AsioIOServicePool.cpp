#include "AsioIOServicePool.h"
#include <iostream>
using namespace std;
AsioIOServicePool::AsioIOServicePool(std::size_t size):_ioServices(size),
_works(size), _nextIOService(0){
	for (std::size_t i = 0; i < size; ++i) {
		_works[i] = std::unique_ptr<Work>(new Work(_ioServices[i]));	//work的构造函数参数就是一个io_context，绑定到该上下文上，防止之后ioService跑起来了直接返回
	}																	//用_works[i] = std::make_unique<Work>(_ioServices[i])比自己手动new一个对象更好

	//遍历多个ioservice，创建多个线程，每个线程内部启动ioservice
	for (std::size_t i = 0; i < _ioServices.size(); ++i) {
		_threads.emplace_back([this, i]() {	// 1、emplace_back开销比push_back小，功能相似。 2、emplace_back进去的是每个线程要执行的函数(用的lambda)，就是让上下文跑起来
			_ioServices[i].run();	// 在第i个线程中让第i个上下文run，从而让每个上下文都跑在单独的线程里
			});
	}
}

AsioIOServicePool::~AsioIOServicePool() {
	Stop();	
	std::cout << "AsioIOServicePool destruct" << endl;
}

boost::asio::io_context& AsioIOServicePool::GetIOService() {	// 返回下一个ioService，同时要更新_nextIOService的索引，让其永远指向下一个上下文
	auto& service = _ioServices[_nextIOService++];
	if (_nextIOService == _ioServices.size()) {
		_nextIOService = 0;
	}
	return service;
}

void AsioIOServicePool::Stop(){
	//因为仅仅执行work.reset并不能让iocontext从run的状态中退出
	//当iocontext已经绑定了读或写的监听事件后，还需要手动stop该服务。
	for (auto& work : _works) {
		//把服务先停止
		work->get_io_context().stop();	// 先让work绑定的上下文停止服务
		work.reset();	// reset后会变为空指针，它绑定的上下文也会回收掉（所以要先停止上下文的服务）
	}

	for (auto& t : _threads) {
		t.join();	// 等待所有线程完成到这一步
	}
}
