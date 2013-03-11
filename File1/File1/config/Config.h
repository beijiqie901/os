#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <map>
using namespace std;

class Config
{
public :
	bool open(string);

	/*
		parameter: string[0]=访问变量;
		function: 获取指定语言文件中的提示信息;
	*/
	string getItem(string);	

private :
	/*
		parameter: string[0]=配置文件路径,string[1]=文件中的标志名称;
		function: 获取指定语言文件路径;
	*/
	string getLanguageFile(string);	

	/*
		parameter: string[0]=特定语言信息文件路径;
		function: 将信息加载到内存;
	*/
	bool copyItems(string);	
	bool getNode(string,string &,string &);
	map<string,string> nodes;
	string languageTag;

};
#endif