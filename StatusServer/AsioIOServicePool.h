#pragma once
#include <vector>
#include <boost/asio.hpp>
#include "Singleton.h"
class AsioIOServicePool:public Singleton<AsioIOServicePool>	// io_context����ʱ���ֽ�IOService
{
	friend Singleton<AsioIOServicePool>;
public:
	using IOService = boost::asio::io_context;	// ��io_contextȡ���ϵ�IOService������
	using Work = boost::asio::io_context::work;	// io_context����io_context.run()ʱ���û�а��κ��¼���ֱ�ӷ��أ����������һ��Work�Ͳ����˳�
	using WorkPtr = std::unique_ptr<Work>;	// ��work��װ��unique_ptr��
	~AsioIOServicePool();
	AsioIOServicePool(const AsioIOServicePool&) = delete;
	AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;
	// ʹ�� round-robin �ķ�ʽ����һ�� io_service
	boost::asio::io_context& GetIOService();	// ����һ��������
	void Stop();
private:
	AsioIOServicePool(std::size_t size = 2/*std::thread::hardware_concurrency()*/);	// �����๹�캯����˽����,������std::thread::hardware_concurrency()����cpu�����������߳�
	std::vector<IOService> _ioServices;	//	���������
	std::vector<WorkPtr> _works;	//�ж���IOService���ж���WorkPtr����ֹ������δ���¼�ʱֱ���˳�
	std::vector<std::thread> _threads;	// �ж��������ľ��ж����̣߳�ÿ�������Ķ����ڵ������߳���
	std::size_t  _nextIOService;	// ��һ��IOService������
};

