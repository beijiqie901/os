#ifndef DIRMANAGER_H
#define DIRMANAGER_H

#include "hardware/struct.h"
#include "hardware/SpaceManager.h"
#include <string>
#include <cstring>
#include <iostream>
#include <time.h>

using namespace std;

class DirManager
{

public:
	DirManager();
	~DirManager();
	///*
	char newFile(string,bool);	//bool：是否为文件夹
	bool deleteDir(string);
	char changeDir(string);	/* input:path; */
	void printChild();
	FCB * getFCB(string);


protected :
	void deleteFile(unsigned int);
	int find(string);	/* 文件的物理序列位置 */
	int findInode(string);

	unsigned int getFileSize(unsigned int);
	SpaceManager * sm;
	Inode* root;
	FCB fcb;
};

DirManager::DirManager()
{
	sm=SpaceManager::getInstance();
	root=sm->getRoot();
	fcb.index=new IndexNode;
	fcb.inode=new Inode;
	*(fcb.inode)=*root;
	fcb.index->i=0;
}
DirManager::~DirManager()
{
	delete(fcb.index);
	delete(fcb.inode);
	delete(root);
}

char DirManager::newFile(string filename,bool isDir)
{
	/* write current inode to new child dir */
	if(find(filename)>=0)
		return 1;
	SpaceBlock *newBlock=sm->request(0);
	if(newBlock==NULL)
		return 2;
	if(isDir)
	{
		IndexNode indexNode;
		char str[3]="..";
		strcpy_s(indexNode.name,str);
		indexNode.i=fcb.index->i;
		newBlock->write(&indexNode);
	}

	/* init new file inode */
	Inode inode;
	inode.first=newBlock->getPage();
	delete(newBlock);
	if(isDir)
	{
		inode.length=1;
		inode.type=1;
	}
	else
	{
		inode.length=0;
		inode.type=0;
	}
	time(&inode.time); //获取从1970至今经过的秒数
	unsigned int i=sm->insertInode(inode);
	//if(i==0) wrong!!

	/* init file indexNode */
	IndexNode newIndexNode;
	newIndexNode.i=i;
	strcpy_s(newIndexNode.name,filename.data());

	/* write file indexNode to disk*/
	unsigned int prePage;
	unsigned int fileSize=getFileSize(fcb.inode->length);
	SpaceBlock *block=sm->get(fcb.inode->first,fileSize,prePage);
	/* 当获取目录页失败时，回滚 */
	if(block==NULL)
	{
		sm->revert(0,inode.first,false);
		inode.type=-1;
		sm->updateInode(inode,newIndexNode.i);
		return 2;

	}
	if(!block->write(&newIndexNode))
	{
		unsigned int lastPage=block->getPage();
		delete(block);
		block=sm->request(lastPage);
		if(block==NULL)
			return 2;
		block->write(&newIndexNode);
		delete(block);
		//unsigned int pageCount=fcb.inode->length/block->getSize()+1;
	}
	fcb.inode->length++;
	sm->updateInode(*fcb.inode,fcb.index->i);
	return 0;
}
char DirManager::changeDir(string filename)
{
	int i=findInode(filename);
	if(i<0) return -1;
	Inode *inode=sm->getInode(i);
	if(inode->type!=1)
		return 1;
	*(fcb.inode)=*inode;
	fcb.index->i=i;
	strcpy_s(fcb.index->name,filename.data());
	delete(inode);
	return 0;
}
bool DirManager::deleteDir(string filename)
{
	int point=find(filename);
	if(point<1) return false;
	unsigned int prePage;
	unsigned int fileSize=getFileSize(point);
	SpaceBlock *cBlock=sm->get(fcb.inode->first,fileSize,prePage);
	IndexNode cIndexNode;
	cBlock->read(&cIndexNode);
	deleteFile(cIndexNode.i);	/* 删除子目录和i节点 */
	
	fileSize=getFileSize(fcb.inode->length-1);
	SpaceBlock *endBlock=sm->get(fcb.inode->first,fileSize,prePage);
	
	/* 非最后一个文件，将最后一个节点覆盖当前节点 */
	if(point+1<(int)fcb.inode->length)
	{
		IndexNode endIndexNode;

		/* 读出最后一个节点*/
		endBlock->read(&endIndexNode);

		/* 将最后一个节点内容写到删除点 */
		unsigned int offset=cBlock->tellg()-sizeof(IndexNode);
		cBlock->seekg(offset);	
		cBlock->write(&endIndexNode);
	}
	
	/* 若为空，归还该页面 */
	if(fileSize%SpaceBlock::getSize()==0)
	{
		sm->revert(prePage,endBlock->getPage(),false);
	}
	delete(endBlock);
	delete(cBlock);

	fcb.inode->length--;
	sm->updateInode(*fcb.inode,fcb.index->i);
	return true;
}
void DirManager::deleteFile(unsigned int i)
{
	Inode * inode=sm->getInode(i);
	if(inode->type==1&&inode->length>1)
	{
		SpaceBlock *block;
		IndexNode indexNode;
		int iPage=inode->first;
		unsigned int fileCount=inode->length;
		unsigned int k=1;
		block=sm->get(iPage);
		block->seekg(sizeof(IndexNode));
		do
		{
			while(block->read(&indexNode)&&k<fileCount)
			{
				deleteFile(indexNode.i);
				k++;
			}
			delete(block);
			iPage=sm->nextPage(iPage);
			if(iPage<=1)	//?<=1
				break;
			block=sm->get(iPage);
		}while(true);	
	}
	inode->type=-1;
	sm->updateInode(*inode,i);
	sm->revert(0,inode->first,true);
	delete(inode);

}

void DirManager::printChild()
{
	int iPage=fcb.inode->first;
	SpaceBlock *block;
	Inode* inode;
	IndexNode indexNode;
	unsigned int fileCount=fcb.inode->length;
	unsigned int k=0;
	do
	{
		block=sm->get(iPage);
		while(block->read(&indexNode)&&k<fileCount)
		{
			inode=sm->getInode(indexNode.i);
			string ts(ctime(&inode->time));
			unsigned int length=inode->length;
			if(inode->type==1)
				length=this->getFileSize(length);
			cout<<indexNode.name<<"\t"<<(int)inode->type;
			cout<<"\t"<<indexNode.i<<"\t"<<inode->first;
			cout<<"\t"<<length<<"\t"<<ts.substr(0,ts.size()-1)<<endl;
			delete(inode);
			k++;
		}
		delete(block);
		iPage=sm->nextPage(iPage);
	}while(iPage>1);
}
FCB * DirManager::getFCB(string filename)
{
	int i=findInode(filename);
	if(i<0)
		return NULL;
	FCB * rf=new FCB;
	IndexNode * indexNode=new IndexNode;
	strcpy_s(indexNode->name,filename.data());
	indexNode->i=i;
	rf->index=indexNode;
	rf->inode=sm->getInode(i);
	return rf;
}

int DirManager::find(string filename)
{
	if(fcb.inode->type==-1)return -1;
	int iPage=fcb.inode->first;
	SpaceBlock *block;
	IndexNode indexNode;
	unsigned int fileCount=fcb.inode->length;
	unsigned int k=0;
	do
	{
		block=sm->get(iPage);
		while(block->read(&indexNode)&&k<fileCount)
		{
			if(indexNode.name==filename)
			{
				delete(block);
				return k;	// the only differece from findInode
			}
			k++;
		}
		delete(block);
		iPage=sm->nextPage(iPage);
	}while(iPage>1);
	return -1;
}

int DirManager::findInode(string filename)
{
	if(fcb.inode->type==-1)return -1;
	int iPage=fcb.inode->first;
	SpaceBlock *block;
	IndexNode indexNode;
	unsigned int fileCount=fcb.inode->length;
	unsigned int k=0;
	do
	{
		block=sm->get(iPage);
		while(block->read(&indexNode)&&k<fileCount)
		{
			if(indexNode.name==filename)
			{
				delete(block);
				return indexNode.i;
			}
			k++;
		}
		delete(block);
		iPage=sm->nextPage(iPage);
	}while(iPage>1);
	return -1;
}

unsigned int DirManager::getFileSize(unsigned int fileLength)
{
	unsigned int blockSize=SpaceBlock::getSize();
	unsigned int iCount=blockSize/sizeof(IndexNode);
	unsigned int iPage=fileLength/iCount;
	unsigned int iOffset=fileLength%iCount;
	unsigned int fileSize=iPage*blockSize+iOffset*sizeof(IndexNode);
	return fileSize;
}

#endif

