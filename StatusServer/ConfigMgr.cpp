#include "ConfigMgr.h"
#include <fstream>  
#include <boost/property_tree/ptree.hpp>  
#include <boost/property_tree/ini_parser.hpp>  
#include <boost/filesystem.hpp>    
#include <map>
#include <iostream>

ConfigMgr::ConfigMgr() {
	boost::filesystem::path current_path = boost::filesystem::current_path();	// 读取当前路径,config.ini就在当前路径
	boost::filesystem::path config_path = current_path / "config.ini";	//boost::filesystem::path类中重载了“/”运算符，将其用作文件路径分隔符
	std::cout << "config path: " << config_path << std::endl;
	// 处理ini文件
	boost::property_tree::ptree pt;	//pt是一个用于存储配置数据的树结构，最上面一层是各section，下一层是section中的键值对
	boost::property_tree::read_ini(config_path.string(), pt);	//读取config.ini文件中的内容，解析并保存到pt中
	for (const auto& section_pair : pt) {
		const std::string& section_name = section_pair.first;	// 提取section的名字，如GateServer、VerifyServer
		const boost::property_tree::ptree section_tree = section_pair.second;
		std::map<std::string, std::string> section_config;
		for (const auto& key_value_pair : section_tree) {
			const std::string& key = key_value_pair.first;
			const std::string& value = key_value_pair.second.get_value<std::string>();	//key_value_pair的second也是个ptree，但ptree类中有个get_value<std::string>()能转成string类型
			section_config[key] = value;
		}

		SectionInfo sectionInfo;
		sectionInfo._section_datas = section_config;	//将这一层section的所有键值对都存入sectionInfo中
		_config_map[section_name] = sectionInfo;	// 将这一层section的名字和保存的sectionInfo存入_config_map中
	}

	// 输出所有的section和key-value对  
	for (const auto& section_entry : _config_map) {
		const std::string& section_name = section_entry.first;
		SectionInfo section_config = section_entry.second;
		std::cout << "[" << section_name << "]" << std::endl;
		for (const auto& key_value_pair : section_config._section_datas) {
			std::cout << key_value_pair.first << "=" << key_value_pair.second << std::endl;
		}
	}

}


