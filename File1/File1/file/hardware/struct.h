#ifndef STRUCT_H
#define STRUCT_H

/* 引导启动 */	
typedef struct Block0
{
	//char information[200];
	unsigned int blockSize;
	unsigned int blockCount;
	unsigned int minFatPage;
	unsigned int fatPageCount;
	unsigned int minInodePage;
	unsigned int inodePageCount;
	unsigned int rootInode;
}Block0;

/* 目录节点，用于快速搜索 */
typedef struct IndexNode
{
	char name[60];
	unsigned int i;	/*i节点*/
}DirIndex;

/* i节点，存储文件详细信息*/
typedef struct Inode
{
	time_t time;
	unsigned int first;	/*文件起始块号*/
	unsigned int length;
	char type;	/* 0 为普通文件，1 为目录，-1初始化值*/
}AttributeNode;

typedef struct FCB
{
	IndexNode* index;
	Inode* inode;
}FCB;

#endif

