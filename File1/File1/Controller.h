#ifndef	CONTROLLER_H
#define CONTROLLER_H

#define EXIT -1
#define SUCCESS 0
#define FAIL 1
#define DO_NOTHING 2

#include "config/Config.h"
#include "file/FileManager.h"
#include "file/DirManager.h"
#include "file/DiskManager.h"
#include <vector>
#include <string>
#include <iostream>
using namespace std;
//enum V{format,open;cd,mkdir,create,del,write,cat;ls,print,flush,close,cls};
class Controller
{
public :
	Controller();
	~Controller();
	char exec(string);
	char init(string);
	void printCurrent();
protected :
	bool filter(string,string&,string&);
	char execute(string);
	char execute(string,string);


	bool isReady;
	string defaultFilename;
	DirManager * dm;
	Config config;
	vector<string> path;

	//const static char[12] v={1,2,3,4,5,6,7,8,9,10,11,12};

};

Controller::Controller()
{
	isReady=false;
	defaultFilename="filesystem";
	dm=NULL;
	config.open("config.txt");
	path.push_back("root");

	cout<<config.getItem("main.hello")<<endl;
	/*
	DiskManager::format("f",1024);
	dm->newFile("America",true);
	dm->newFile("China",true);
	dm->newFile("England",true);
	dm->printChild();

	dm->changeDir("England");
	cout<<"change to England"<<endl;
	dm->newFile("lius",true);
	dm->newFile("hans",true);
	dm->newFile("t",false);
	dm->printChild();	
	*/
}
Controller::~Controller()
{
	if(this->dm!=NULL)
		delete(this->dm);
}

char Controller::exec(string command)
{
	string sa,sb;
	if(!filter(command,sa,sb))
		return DO_NOTHING;
	if(sb=="")
	{
		return execute(sa);
	}
	else
	{
		return execute(sa,sb);
	}


}


char Controller::init(string command)
{
	string sa,sb;
	if(!filter(command,sa,sb))
		return DO_NOTHING;

	if(sb=="")
	{
		if(sa=="close")
		{
			cout<<config.getItem("main.exit")<<endl;
			return EXIT;
		}
		else
		{
			cout<<config.getItem("error.noinstance")<<endl;
			return DO_NOTHING;
		}
	}
	else if(sa=="open")
	{
		isReady=DiskManager::open(sb);
		if(isReady)
		{
			dm=new DirManager;
			return SUCCESS;
		}
		else
		{
			cout<<config.getItem("error.file.failopen")<<endl;
			return FAIL;
		}
	}
	else if(sa=="format")
	{
		int blockSize=atoi(sb.data()); 
		if(blockSize==0)
		{
			cout<<config.getItem("error.file.number")<<endl;
			return DO_NOTHING;
		}
		isReady=DiskManager::format(defaultFilename,blockSize);
		if(isReady)
		{
			dm=new DirManager;
			return SUCCESS;
		}
		else
		{
			cout<<config.getItem("error.file.failformat")<<endl;
			return FAIL;
		}
	}
	else
	{
		cout<<config.getItem("error.noinstance")<<endl;
		return DO_NOTHING;
	}
}

void Controller::printCurrent()
{
	cout<<"> ";
	for(int i=0;i<(int)path.size();i++)
	{
		cout<<path[i]<<'\\';
	}
	cout<<">";
}
char Controller::execute(string sa,string sb)
{
	if(!isReady)
	{
		cout<<config.getItem("error.noinstance")<<endl;
		return DO_NOTHING;
	}

	if(sa=="mkdir")
	{
		char ch=dm->newFile(sb,true);
		if(ch==1)
		{
			cout<<config.getItem("error.samename")<<endl;
			return FAIL;
		}
		else if(ch==2)
		{
			cout<<config.getItem("error.nospace")<<endl;
			return FAIL;
		}
		return SUCCESS;
	}
	else if(sa=="create")
	{
		char ch=dm->newFile(sb,false);
		if(ch==1)
		{
			cout<<config.getItem("error.samename")<<endl;
			return FAIL;
		}
		else if(ch==2)
		{
			cout<<config.getItem("error.nospace")<<endl;
			return FAIL;
		}
		return SUCCESS;
	}
	else if(sa=="del")
	{
		if(dm->deleteDir(sb))
			return SUCCESS;
		else 
		{
			cout<<'\''<<sb<<'\''<<config.getItem("error.nofile")<<endl;
			return FAIL;
		}
	}
	else if(sa=="cd")
	{
		char ch=dm->changeDir(sb);
		if(ch==-1)
		{
			cout<<config.getItem("error.nofile")<<endl;
			return FAIL;
		}
		else if(ch==1)
		{
			cout<<config.getItem("error.nocd")<<endl;
			return FAIL;
		}

		if(sb=="..")
		{
			if(path.size()>1)
				path.pop_back();
		}
		else 
			path.push_back(sb);
		return SUCCESS;
	}

	else if(sa=="cat")
	{
		FCB * fcb=dm->getFCB(sb);
		if(fcb==NULL)
		{
			cout<<config.getItem("error.nofile")<<endl;
			return FAIL;
		}
		else if(fcb->inode->type!=0)
		{
			cout<<config.getItem("error.noread")<<endl;
			return FAIL;
		}
		unsigned int length=fcb->inode->length;
		char *str=new char[length+1];
		FileManager fm(fcb);
		unsigned int n=fm.read(str,length);
		str[n]='\0';
		cout<<str<<endl;

		delete(str);
		delete(fcb->index);
		delete(fcb->inode);
		delete(fcb);
		return SUCCESS;
	}	
	else if(sa=="write")
	{
		bool append=false;
		unsigned int offset=0;
		string wsa,wsb;
		filter(sb,wsa,wsb);
		if(wsb=="a") append=true;
		else offset=atoi(wsb.data()); 
		FCB * fcb=dm->getFCB(wsa);
		if(fcb==NULL)
		{
			cout<<config.getItem("error.nofile")<<endl;
			return FAIL;
		}
		else if(fcb->inode->type!=0)
		{
			cout<<config.getItem("error.nowrite")<<endl;
			return FAIL;
		}
		unsigned int length=fcb->inode->length;
		
		FileManager fm(fcb);
		if(append)
		{
			char *str=new char[length+1];
			unsigned int n=fm.read(str,length);
			str[n]='\0';
			cout<<str;
			delete(str);
		}
		string text;
		cin.clear();
		/*
		char ch;
		while(cin>>ch,!cin.eof())
		{
			text.append(1,ch);
		}
		*/
		string str;
		while(!cin.eof())
		{
			//gets_s(str);
			getline(cin, str);
			text.append(str+'\n');
		}
		unsigned int n;
		if(offset==0)
			n=fm.write((char*)text.data(),text.length());
		else 
			n=fm.write((char*)text.data(),text.length(),offset);
		if(n<text.length())
		{
			cout<<config.getItem("warning.nowriteall")<<endl;
		}
		delete(fcb->index);
		delete(fcb->inode);
		delete(fcb);
		return SUCCESS;
	}
	else
	{
		cout<<'\''<<sa<<' '<<sb<<"' "<<config.getItem("error.nocommand")<<endl;
		return DO_NOTHING;
	}
}



char Controller::execute(string sa)
{
	if(!isReady)
	{
		cout<<config.getItem("error.noinstance")<<endl;
		return DO_NOTHING;
	}

	if(sa=="ls")
	{
		cout<<config.getItem("item.filename");
		cout<<"\t"<<config.getItem("item.type");
		cout<<"\t"<<config.getItem("item.inode");
		cout<<"\t"<<config.getItem("item.first");
		cout<<"\t"<<config.getItem("item.length");
		cout<<"\t"<<config.getItem("item.time")<<endl;
		dm->printChild();
	}
	else if(sa=="cls")
	{
		system("cls"); 
	}
	else if(sa=="print")
	{
		DiskManager::print();
	}
	else if(sa=="flush")
	{
		DiskManager::flush();
	}
	else if(sa=="close")
	{
		DiskManager::flush();
		cout<<config.getItem("main.exit")<<endl;
		return EXIT;
	}
	else if(sa=="")
	{
		return DO_NOTHING;
	}
	else
	{
		cout<<'\''<<sa<<"' "<<config.getItem("error.nocommand")<<endl;
		return DO_NOTHING;
	}
	return SUCCESS;

}

bool Controller::filter(string command,string& sa,string& sb)
{
	sa="";
	sb="";
	int ia=-1,ib;
	int size=(int)command.size();
	while(++ia<size && (command.at(ia)==' '||command.at(ia)=='\t'));
	if(ia>=size)	/* ÎÞ¿É¼û×Ö·û */
		return false;
	ib=command.find(' ',ia);
	if(ib>0)
	{
		sa=command.substr(ia,ib-ia);
		ia=ib;
		while(++ia<size &&  (command.at(ia)==' '||command.at(ia)=='\t'));
		if(ia<size)
		{
			ib=size;
			while(--ib<size &&  (command.at(ib)==' '||command.at(ib)=='\t'));
			ib++;
			sb=command.substr(ia,ib-ia);
		}
	}
	else
	{
		sa=command.substr(ia);
	}
	return true;
}
#endif