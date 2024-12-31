#pragma once
#include "const.h"

struct SectionInfo {	//比如config.ini中的[GateServer]就是个Section，里面有一些键值对
	SectionInfo(){}
	~SectionInfo() { _section_datas.clear(); }

	SectionInfo(const SectionInfo& src) {	//拷贝构造
		_section_datas = src._section_datas;
	}

	SectionInfo& operator=(const SectionInfo& src) {	// 重载赋值运算符
		if (&src == this) return *this;	// 不能忽略拷贝自己的情况
		_section_datas = src._section_datas;
	}

	// 用map来保存section中的键值对
	std::map<std::string, std::string> _section_datas;
	//重载[]，就能通过SectionInfo[key]的方式来读取值
	std::string operator[](const std::string& key) {
		if (_section_datas.find(key) == _section_datas.end()) {
			std::cout << "no keys named "<<key << std::endl;
			return "";
		}
		return _section_datas[key];
	}

};

//config.ini配置信息的管理类
class ConfigMgr	// 在配置多线程时将其改为了单例类
{
public:
	~ConfigMgr() {
		_config_map.clear();
	}

	SectionInfo operator[](std::string sectionName) {
		if (_config_map.find(sectionName) == _config_map.end()) {
			std::cout << "No section called " << sectionName << std::endl;
			return SectionInfo();	// 返回一个空的SectionInfo结构体
		}
		return _config_map[sectionName];
	}
	
	static ConfigMgr& getInst() {	// 另一种生成单例类的方法（C++11以上），外部通过Inst()函数获取单例
		static ConfigMgr cfg_mgr;	// cfg_mgr由于是static的，被多次访问也只初始化一次，生命周期持续到进程结束
		return cfg_mgr;
	}

	ConfigMgr(const ConfigMgr& src) {
		_config_map = src._config_map;
	}

	ConfigMgr& operator=(const ConfigMgr& src) {
		if (&src == this) return *this;
		_config_map = src._config_map;
		return *this;
	}
private:
	ConfigMgr();
	std::map <std::string, SectionInfo> _config_map;
};

