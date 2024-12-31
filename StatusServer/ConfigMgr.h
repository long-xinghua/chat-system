#pragma once
#include "const.h"

struct SectionInfo {	//����config.ini�е�[GateServer]���Ǹ�Section��������һЩ��ֵ��
	SectionInfo(){}
	~SectionInfo() { _section_datas.clear(); }

	SectionInfo(const SectionInfo& src) {	//��������
		_section_datas = src._section_datas;
	}

	SectionInfo& operator=(const SectionInfo& src) {	// ���ظ�ֵ�����
		if (&src == this) return *this;	// ���ܺ��Կ����Լ������
		_section_datas = src._section_datas;
	}

	// ��map������section�еļ�ֵ��
	std::map<std::string, std::string> _section_datas;
	//����[]������ͨ��SectionInfo[key]�ķ�ʽ����ȡֵ
	std::string operator[](const std::string& key) {
		if (_section_datas.find(key) == _section_datas.end()) {
			std::cout << "no keys named "<<key << std::endl;
			return "";
		}
		return _section_datas[key];
	}

};

//config.ini������Ϣ�Ĺ�����
class ConfigMgr	// �����ö��߳�ʱ�����Ϊ�˵�����
{
public:
	~ConfigMgr() {
		_config_map.clear();
	}

	SectionInfo operator[](std::string sectionName) {
		if (_config_map.find(sectionName) == _config_map.end()) {
			std::cout << "No section called " << sectionName << std::endl;
			return SectionInfo();	// ����һ���յ�SectionInfo�ṹ��
		}
		return _config_map[sectionName];
	}
	
	static ConfigMgr& getInst() {	// ��һ�����ɵ�����ķ�����C++11���ϣ����ⲿͨ��Inst()������ȡ����
		static ConfigMgr cfg_mgr;	// cfg_mgr������static�ģ�����η���Ҳֻ��ʼ��һ�Σ��������ڳ��������̽���
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

