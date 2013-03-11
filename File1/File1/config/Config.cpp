#include "stdafx.h"
#include "Config.h"
#include <fstream>
#include <iostream>
using namespace std;


bool Config::open(string configFile)
{
	languageTag="language";
	string languageFile=this->getLanguageFile(configFile);
	if(languageFile=="")
		return false;
	return this->copyItems(languageFile);
}


string Config::getItem(string tag)
{
	map<string,string>::iterator p=nodes.find(tag);
	if(p==nodes.end())
		return "--NO '"+tag+"'!";
	else 
		return p->second;
}


string Config::getLanguageFile(string configFile)
{
	ifstream input;
	input.open(configFile.data());
	char line[100];
	while(!input.eof())
	{
		input.getline(line,100,'\n');
		string tag,value;
		if(getNode((string)line,tag,value)&&tag==this->languageTag)
			return value;
	}
	return "";
	
}
bool Config::copyItems(string languageFile)
{
	ifstream input;
	input.open(languageFile.data());
	char line[100];
	while(!input.eof())
	{
		input.getline(line,100,'\n');
		string tag,value;
		if(getNode((string)line,tag,value))
			nodes.insert(map<string,string>::value_type(tag,value));
	}
	return true;
}

bool Config::getNode(string line,string &tag,string &value)
{
	int index=line.find('=');
	if(index>0)
	{
		int i,j;

		i=-1;
		j=index;
		while(line.at(++i)==' ');
		while(line.at(--j)==' ');
		tag=line.substr(i,j-i+1);

		i=index;
		j=line.length();
		while(line.at(++i)==' ');
		while(line.at(--j)==' ');
		value=line.substr(i,j-i+1);
		return true;
	}
	return false;
}


