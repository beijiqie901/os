#ifndef SPACEMANAGER_H
#define SPACEMANAGER_H
#define CAPACITY 102400
#include <iostream>

#include "SpaceBlock.h"
#include "struct.h"
#include <string>
#include <vector>
using namespace std;
/*
	-- > ʵʱ����fat��disk
ʹ��-- > flush()���µ�disk��
*/
class SpaceManager
{
private:
	SpaceManager();	//single model
public:
	static SpaceManager* getInstance();
	~SpaceManager();
	unsigned int requestPage(unsigned int);	// current page;
	SpaceBlock* request(unsigned int);	// current page;
	SpaceBlock* get(unsigned int);	// current page;
	SpaceBlock* get(unsigned int,unsigned int,unsigned int&);	//firstPage,length, prePage;
	unsigned int nextPage(unsigned int);		// current page;
	bool revert(unsigned int, unsigned int, bool);	// prePage, revert page,isLinkPage;
	
	bool format(string,unsigned int);	// filename,blockCount;
	bool open(string);	// filename;
	bool flush();

	Inode* getRoot();
	unsigned int insertInode(Inode&);
	bool updateInode(Inode&,unsigned int);
	Inode* getInode(unsigned int);
	bool deleteInode(unsigned int);

	void printIndex();

protected :
	///*
	bool updateIndex(unsigned int,unsigned int);	// i, value

	bool readInode(unsigned int,unsigned int,unsigned int);
	unsigned int initInode(unsigned int,unsigned int);
	
	bool readFAT(unsigned int,unsigned int,unsigned int);
	unsigned int initFAT(unsigned int,unsigned int);

	char * getBlockBegin(unsigned int);	//page;

	char *content;
	/*
		��0,1����ʹ��;
		Ϊ0ʱ��ʾδʹ��;
		Ϊ1ʱ��ʾ����һ����;
		����ֵ��ʾ��������һ������
	*/
	unsigned int *index;
	bool *inodeIndex;	/* �Ƿ�ռ�� */
	static SpaceManager* instance;

	Block0 block0;
	string filename;
	unsigned int minUseablePage;	//inode
	unsigned int lastRequestPage;
	unsigned int lastRequestInode;
};

#endif

