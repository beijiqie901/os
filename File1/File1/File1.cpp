// File1.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "Controller.h"
#include <time.h>
#include <iostream>
#include <string>
using namespace std;

int main()
{
	Controller con;
	char str[200];
	while(true)
	{
		gets_s(str);
		//getline(cin, command);
		///cout<<command<<endl;
		string command(str);
		char ch=con.init(command);
		if(ch==SUCCESS)
		{
			con.printCurrent();
			break;
		}
		if(ch==EXIT)
		{
			getchar();
			return 0;
		}
	}
	while(true)
	{
		gets_s(str);
		//getline(cin, command);
		///cout<<command<<endl;
		string command(str);
		char ch=con.exec(command);
		if(ch==EXIT)
		{
			getchar();
			return 0;
		}
		con.printCurrent();
	}
}