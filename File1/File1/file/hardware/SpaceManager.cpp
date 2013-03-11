#include "stdafx.h"
#include "SpaceManager.h"
#include <iostream>
#include <fstream>
#include <time.h>
using namespace std;

SpaceManager* SpaceManager::instance=0;
SpaceManager::SpaceManager()/* single model */
{
	/* init content to empty! */
	this->content=new char[CAPACITY];
	for(unsigned long j=0;j<CAPACITY;j++)
	{
		content[j]=0;
	}

	this->index=NULL;
	this->inodeIndex=NULL;
	lastRequestInode=0;

}	
SpaceManager* SpaceManager::getInstance()
{
	if(SpaceManager::instance==0)
	{
		SpaceManager::instance=new SpaceManager;
	}
	return SpaceManager::instance;
}

SpaceManager::~SpaceManager()
{
	delete(this->instance);
	delete(this->content);
	delete(this->index);
	delete(this->inodeIndex);
}

//OK
unsigned int SpaceManager::requestPage(unsigned int currentPage)	
{
	if(currentPage>=block0.blockCount)/* 不允许获取 */
		return 0;
	unsigned int requestPage=lastRequestPage;
	while(++requestPage!=lastRequestPage)
	{
		if(requestPage>=block0.blockCount)
			requestPage=minUseablePage;
		if(index[requestPage]==0)
			break;
	}
	/* not enough space*/
	if(requestPage==lastRequestPage&&index[requestPage]>0)
		return 0;
	/* link next requestPage to currentPage*/
	if(currentPage>=this->minUseablePage)
	{
		index[currentPage]=requestPage;
		this->updateIndex(currentPage,requestPage);
	}
	index[requestPage]=1;
	this->updateIndex(requestPage,1);
	lastRequestPage=requestPage;
	return requestPage;
}
SpaceBlock* SpaceManager::request(unsigned int currentPage)	
{
	unsigned int requestPage=this->requestPage(currentPage);

	if(requestPage<this->minUseablePage)
		return NULL;
	char *ch=getBlockBegin(requestPage);
	SpaceBlock *block=new SpaceBlock(requestPage,ch);
	return block;
}

//OK
SpaceBlock* SpaceManager::get(unsigned int page)
{
	if(page<minUseablePage||page>=block0.blockCount)/* 不允许获取 */
		return NULL;
	char *ch=getBlockBegin(page);
	SpaceBlock *block=new SpaceBlock(page,ch);
	return block;
}

//OK
SpaceBlock* SpaceManager::get(unsigned int firstPage,unsigned int length,unsigned int & prePage)
{
	prePage=0;
	unsigned int ipage=length/this->block0.blockSize; // be care
	unsigned int offset=length%block0.blockSize;
	unsigned int cpage=firstPage,i=0;
	while(i<ipage)
	{
		i++;
		if(index[cpage]<minUseablePage)
		{
			if(i==ipage)
			{
				cpage=this->requestPage(cpage);
				if(cpage<this->minUseablePage)
					return NULL;
				break;
			}
			else
				return NULL;
		}
		prePage=cpage;
		cpage=index[cpage];
	}

	char *ch=getBlockBegin(cpage);
	SpaceBlock *block=new SpaceBlock(cpage,ch);
	block->seekg(offset);
	return block;
}

// OK
unsigned int SpaceManager::nextPage(unsigned int currentPage)
{
	if(currentPage<minUseablePage||currentPage>=block0.blockCount)
	{
		return -1;		/* 非法 */ 
	}
	return index[currentPage];
}

// OK
bool SpaceManager::revert(unsigned int prePage,unsigned int revertPage, bool isLinkPage)
{
	if(revertPage<minUseablePage||revertPage>=block0.blockCount||prePage>=block0.blockCount)
		return false;	/* 非法 */ 

	/* 存在前驱时，连通前后*/
	if(prePage>=minUseablePage)
	{
		if(isLinkPage)
			index[prePage]=1;
		else
			index[prePage]=index[revertPage];
		this->updateIndex(prePage,index[prePage]);
	}
	if(isLinkPage)
	{
		unsigned int nextPage=revertPage,t;
		while(nextPage>=minUseablePage)
		{
			t=index[nextPage];
			index[nextPage]=0;
			this->updateIndex(nextPage,0);
			nextPage=t;
		}
		index[nextPage]=0;
		this->updateIndex(nextPage,0);
	}
	else
	{
		index[revertPage]=0;
		this->updateIndex(revertPage,0);
	}
	return true;
}

//OK
bool SpaceManager::format(string filename,unsigned int blockSize)
{
	unsigned int blockCount=CAPACITY/blockSize;
	if(blockSize<sizeof(unsigned int)||blockCount<4)
		return false;
	this->filename=filename;
	this->block0.blockCount=blockCount;
	this->block0.blockSize=blockSize;
	SpaceBlock::setSize(blockSize);


	/* init block,fat,inode */
	unsigned int fatb=1;
	unsigned int fatc=this->initFAT(fatb,blockCount);
	unsigned int inodeb=fatb+fatc;
	unsigned int inodec=this->initInode(inodeb,blockCount);
	unsigned int useableb=inodeb+inodec;

	this->minUseablePage=useableb;	
	this->lastRequestPage=useableb-1;


	/* write block0 to disk*/
	this->block0.minFatPage=fatb;
	this->block0.minInodePage=inodeb;
	this->block0.fatPageCount=fatc;
	this->block0.inodePageCount=inodec;

	/* init root dir */
	SpaceBlock *sb=this->request(0);
	IndexNode indexNode;
	indexNode.i=0;
	char str[3]="..";
	strcpy_s(indexNode.name,str);
	sb->write(&indexNode);

	/* 初始化根目录 */
	Inode root;
	root.first=sb->getPage();
	delete(sb);
	root.length=1;
	root.type=1;
	time(&root.time); //获取从1970至今经过的秒数
	char *ch=getBlockBegin(inodeb);
	SpaceBlock b1(inodeb,ch);
	if(b1.write(&root))
		this->inodeIndex[0]=true;
	else 
		return false;

	this->block0.rootInode=0;
	
	SpaceBlock b2(0,content);
	return (b2.write(&block0)>0);

}
bool SpaceManager::open(string filename)
{
	ifstream input;
	input.open(filename.data());
	if(!input.is_open())
		return false;

	this->filename=filename;
	input.read(this->content,CAPACITY);

	/* block0 */
	unsigned int blockSize=sizeof(Block0);
	SpaceBlock::setSize(blockSize);
	SpaceBlock block(0,this->content);
	if(block.read(&this->block0)==0)
		return false;
	SpaceBlock::setSize(block0.blockSize);

	/* fat,inode */
	this->readFAT(block0.minFatPage,block0.fatPageCount,block0.blockCount);
	this->readInode(block0.minInodePage,block0.inodePageCount,block0.blockCount);

	minUseablePage=block0.minInodePage+block0.inodePageCount;	
	lastRequestPage=minUseablePage-1;
	return true;
}

bool SpaceManager::flush()
{
	ofstream output;
	output.open(this->filename.data());
	output.write(this->content,CAPACITY);
	return true;
}
Inode* SpaceManager::getRoot()
{
	return this->getInode(block0.rootInode);
}
bool SpaceManager::updateInode(Inode & inode,unsigned int i)
{
	unsigned int t=block0.blockSize/sizeof(Inode);
	unsigned int page=i/t;
	if(page>block0.inodePageCount)
		return false;
	page+=block0.minInodePage;
	int offset=(i%t)*sizeof(Inode);

	char *ch=getBlockBegin(page);
	SpaceBlock block(page,ch);
	if(!block.seekg(offset))
		return false;

	if(block.write(&inode))
	{
		if(inode.type==-1)
			inodeIndex[i]=false;
		return true;
	}
	else 
		return false;
}
unsigned int SpaceManager::insertInode(Inode & inode)
{
	/* find space to write */
	unsigned int t=block0.blockSize/sizeof(Inode);	/*每块可存的inode个数*/
	unsigned int requestPoint=this->lastRequestInode;
	unsigned int staticMaxInodeCount=block0.inodePageCount*t;
	while(++requestPoint!=lastRequestInode)
	{
		if(requestPoint>=staticMaxInodeCount)
			requestPoint=0;
		if(inodeIndex[requestPoint]==false)
			break;
	}
	/* not enough space*/
	if(requestPoint==lastRequestInode&&inodeIndex[requestPoint])
		return NULL;
	this->lastRequestInode=requestPoint;

	unsigned int page=requestPoint/t;
	page+=block0.minInodePage;
	int offset=(requestPoint%t)*sizeof(Inode);

	char *ch=getBlockBegin(page);
	SpaceBlock block(page,ch);
	if(!block.seekg(offset))
		return 0;

	if(block.write(&inode))
	{
		this->inodeIndex[requestPoint]=true;
		return requestPoint;
	}
	else 
		return 0;
}
Inode* SpaceManager::getInode(unsigned int i)
{
	unsigned int t=block0.blockSize/sizeof(Inode);
	unsigned int page=i/t;
	if(page>block0.inodePageCount)
		return NULL;
	page+=block0.minInodePage;
	int offset=(i%t)*sizeof(Inode);
	char *ch=getBlockBegin(page) ;
	SpaceBlock block(page,ch);
	if(!block.seekg(offset))
		return NULL;

	Inode *pInode=new Inode;
	if(!block.read(pInode))
		return NULL ;
	return pInode;
}

bool SpaceManager::deleteInode(unsigned int i)
{
	unsigned int t=block0.blockSize/sizeof(Inode);
	unsigned int page=i/t;
	if(page>block0.inodePageCount)
		return false;
	page+=block0.minInodePage;
	int offset=(i%t)*sizeof(Inode);
	char *ch=getBlockBegin(page);
	SpaceBlock block(page,ch);
	if(!block.seekg(offset))
		return false;

	Inode inode;
	inode.type=-1;
	if(block.write(&inode))
	{
		this->inodeIndex[i]=false;
		return true;
	}
	else 
		return false;
}

///*

bool SpaceManager::readInode(unsigned int page,unsigned int pageCount,unsigned int blockCount)
{
	unsigned int indexSize=pageCount*(block0.blockSize/sizeof(Inode));
	if(inodeIndex==NULL)
	{
		inodeIndex=new bool[indexSize];
		if(inodeIndex==NULL)
			return false;
	}
	Inode inode;
	unsigned int iPage=page+pageCount;
	unsigned int k=0;
	for(unsigned int i=page;i<iPage;i++)
	{
		char *ch=getBlockBegin(i);
		SpaceBlock block(i,ch);
		while(block.read(&inode))
		{
			if(inode.type==0||inode.type==1)
				inodeIndex[k]=true;
			else 
				inodeIndex[k]=false;
			k++;
		}
	}
	return true;
}
unsigned int SpaceManager::initInode(unsigned int page,unsigned int blockCount)
{
	/* init inodeIndex(momery) to empty! */
	this->inodeIndex=new bool[blockCount];
	if(inodeIndex==NULL) return false;	/* ? to free index,content */
	for(unsigned int i=0;i<blockCount;i++)
	{
		inodeIndex[i]=false;
	}

	/* write to disk*/
	unsigned int inodePageCount=(blockCount-1)/(block0.blockSize/sizeof(Inode))+1;	/* 文件数极端情况下为磁盘块数 */
	Inode inode;
	inode.type=-1;
	unsigned int iPage=page+inodePageCount;
	for(unsigned int j=page;j<iPage;j++)
	{
		char *ch=getBlockBegin(j);
		SpaceBlock block(j,ch);
		while(block.write(&inode));
	}
	return inodePageCount;
}


///////////////////////	
bool SpaceManager::readFAT(unsigned int page,unsigned int pageCount,unsigned int blockCount)
{	
	/* init index to empty! */
	if(this->index==NULL)
	{
		this->index=new unsigned int[blockCount];
		if(this->index==NULL) return false;
	}

	unsigned int iPage=page+pageCount;
	unsigned int k=0,value;
	for(unsigned int i=page;i<iPage;i++)
	{
		char *ch=getBlockBegin(i);
		SpaceBlock block(i,ch);
		while(block.read(&value)&&k<blockCount)
		{
			index[k]=value;
			k++;
		}
	}
	return true;
}
unsigned int SpaceManager::initFAT(unsigned int page,unsigned int blockCount)
{
	/* init index to empty! */
	if(this->index==NULL)
	{
		this->index=new unsigned int[blockCount];
		if(this->index==NULL) return false;
	}
	for(unsigned int i=0;i<blockCount;i++)
	{
		index[i]=0;
	}

	/* write to disk */
	unsigned int fatPageCount=(blockCount-1)/(block0.blockSize/sizeof(unsigned int))+1;
	unsigned int iPage=page+fatPageCount;
	unsigned int k=0;
	for(unsigned int j=page;j<iPage;j++)
	{
		char *ch=getBlockBegin(j);
		SpaceBlock block(j,ch);
		while(block.write(index+k)&&k<blockCount)
			k++;
	}
	return fatPageCount;
}

bool SpaceManager::updateIndex(unsigned int i,unsigned int value)
{
	/* fat写入到虚拟磁盘中 */
	unsigned int icount=block0.blockSize/sizeof(value);
	unsigned int page=block0.minFatPage+i/icount;	
	unsigned int offset=(i%icount)*sizeof(value);
	char *ch=getBlockBegin(page);
	SpaceBlock block(page,ch);
	block.seekg(offset);
	return (block.write(&value)>0);
}

char * SpaceManager::getBlockBegin(unsigned int page)
{
	return this->content+page*block0.blockSize;
}
void SpaceManager::printIndex()
{

	for(unsigned int i=0;i<block0.blockCount;i++)
	{
		cout<<index[i]<<" ";
	}
	cout<<endl;

}

