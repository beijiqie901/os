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
		parameter: string[0]=���ʱ���;
		function: ��ȡָ�������ļ��е���ʾ��Ϣ;
	*/
	string getItem(string);	

private :
	/*
		parameter: string[0]=�����ļ�·��,string[1]=�ļ��еı�־����;
		function: ��ȡָ�������ļ�·��;
	*/
	string getLanguageFile(string);	

	/*
		parameter: string[0]=�ض�������Ϣ�ļ�·��;
		function: ����Ϣ���ص��ڴ�;
	*/
	bool copyItems(string);	
	bool getNode(string,string &,string &);
	map<string,string> nodes;
	string languageTag;

};
#endif