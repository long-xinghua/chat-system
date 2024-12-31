#include "ConfigMgr.h"
#include <fstream>  
#include <boost/property_tree/ptree.hpp>  
#include <boost/property_tree/ini_parser.hpp>  
#include <boost/filesystem.hpp>    
#include <map>
#include <iostream>

ConfigMgr::ConfigMgr() {
	boost::filesystem::path current_path = boost::filesystem::current_path();	// ��ȡ��ǰ·��,config.ini���ڵ�ǰ·��
	boost::filesystem::path config_path = current_path / "config.ini";	//boost::filesystem::path���������ˡ�/������������������ļ�·���ָ���
	std::cout << "config path: " << config_path << std::endl;
	// ����ini�ļ�
	boost::property_tree::ptree pt;	//pt��һ�����ڴ洢�������ݵ����ṹ��������һ���Ǹ�section����һ����section�еļ�ֵ��
	boost::property_tree::read_ini(config_path.string(), pt);	//��ȡconfig.ini�ļ��е����ݣ����������浽pt��
	for (const auto& section_pair : pt) {
		const std::string& section_name = section_pair.first;	// ��ȡsection�����֣���GateServer��VerifyServer
		const boost::property_tree::ptree section_tree = section_pair.second;
		std::map<std::string, std::string> section_config;
		for (const auto& key_value_pair : section_tree) {
			const std::string& key = key_value_pair.first;
			const std::string& value = key_value_pair.second.get_value<std::string>();	//key_value_pair��secondҲ�Ǹ�ptree����ptree�����и�get_value<std::string>()��ת��string����
			section_config[key] = value;
		}

		SectionInfo sectionInfo;
		sectionInfo._section_datas = section_config;	//����һ��section�����м�ֵ�Զ�����sectionInfo��
		_config_map[section_name] = sectionInfo;	// ����һ��section�����ֺͱ����sectionInfo����_config_map��
	}

	// ������е�section��key-value��  
	for (const auto& section_entry : _config_map) {
		const std::string& section_name = section_entry.first;
		SectionInfo section_config = section_entry.second;
		std::cout << "[" << section_name << "]" << std::endl;
		for (const auto& key_value_pair : section_config._section_datas) {
			std::cout << key_value_pair.first << "=" << key_value_pair.second << std::endl;
		}
	}

}


