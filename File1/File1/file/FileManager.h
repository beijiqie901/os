#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "hardware/struct.h"
#include "hardware/SpaceManager.h"
#include <string>
#include <cstring>
#include <iostream>
using namespace std;

class FileManager
{
public:
	FileManager(FCB *);
	~FileManager();
	unsigned int write(char *,unsigned int);
	unsigned int write(char *,unsigned int,unsigned int);	// ch, length,point
	unsigned int read(char *,unsigned int);
	unsigned int read(char *,unsigned int,unsigned int);	// ch, length,point

	bool close();
	template <class type>
	unsigned int write(type *value)
	{
		unsigned int typeSize=sizeof(type);
		char *a=reinterpret_cast<char *> (value);
		return write(a,typeSize);
	}

	template <class type>
	unsigned int read(type *value)
	{
		unsigned int typeSize=sizeof(type);
		if((typeSize+this->offset)>(fcb->inode->length))
			return 0;
		char *a=reinterpret_cast<char *> (value);
		return read(a,typeSize);
	}
	
protected :
	bool setPoint(unsigned int);

	SpaceManager * sm;
	FCB * fcb;
	SpaceBlock * block;
	unsigned int offset;	/*文件读写偏移量*/
	unsigned int blockSize;
};

FileManager::FileManager(FCB * fcb)
{
	this->fcb=fcb;
	sm=SpaceManager::getInstance();	
	block=sm->get(fcb->inode->first);
	offset=0;
	blockSize=SpaceBlock::getSize();
}
FileManager::~FileManager()
{
	if(block!=NULL)
		delete(block);
}


unsigned int FileManager::write(char * a,unsigned int typeSize)
{
	unsigned int c=typeSize;
	unsigned int n=block->write(a,c);
	SpaceBlock * tBlock;
	unsigned int nextPage;
	while(n<c)
	{
		tBlock=block;
		/* 若文件存在下一页则覆盖，否则申请新页*/
		nextPage=sm->nextPage(block->getPage());
		block=sm->get(nextPage);
		if(block==NULL)
		{
			block=sm->request(tBlock->getPage());
			if(block==NULL)
				break;
		}
		delete(tBlock);
		c-=n;
		a+=n;
		n=block->write(a,c);
	}
	unsigned int writeChar=typeSize-(c-n);
	this->offset+=writeChar;
	if((this->offset)>(fcb->inode->length))
	{
		fcb->inode->length=this->offset;	/* 更新文件大小 */
		sm->updateInode(*fcb->inode,fcb->index->i);	/* 刷新磁盘 */
	}
	return writeChar;
}
unsigned int FileManager::write(char * a,unsigned int typeSize,unsigned int point)
{
	if(!this->setPoint(point))
		return 0;
	return write(a,typeSize);
}
unsigned int FileManager::read(char * a,unsigned int typeSize,unsigned int point)
{
	if(!this->setPoint(point))
		return 0;
	return write(a,typeSize);
}

unsigned int FileManager::read(char * a,unsigned int typeSize)
{
	unsigned int length=fcb->inode->length;
	if((this->offset+typeSize)>length)
		typeSize=length-offset;
	unsigned int c=typeSize;
	unsigned int n=block->read(a,c);
	SpaceBlock * tBlock;
	unsigned int nextPage;
	while(n<c)
	{
		tBlock=block;
		nextPage=sm->nextPage(block->getPage());
		block=sm->get(nextPage);
		delete(tBlock);
		c-=n;
		a+=n;
		n=block->read(a,c);
	}
	this->offset+=typeSize;
	return typeSize;	
}

bool FileManager::close()
{
	return true;

}

bool FileManager::setPoint(unsigned int point)
{
	if(point>(fcb->inode->length))
		return false;
	if(offset/this->blockSize==point/blockSize)
	{
		block->seekg(point%blockSize);
	}
	else
	{
		delete(block);
		unsigned int prePage;
		block=sm->get(fcb->inode->first,point,prePage);
	}
	this->offset=point;
	return true;
}


#endif

