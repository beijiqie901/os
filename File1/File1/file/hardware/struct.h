#ifndef STRUCT_H
#define STRUCT_H

/* �������� */	
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

/* Ŀ¼�ڵ㣬���ڿ������� */
typedef struct IndexNode
{
	char name[60];
	unsigned int i;	/*i�ڵ�*/
}DirIndex;

/* i�ڵ㣬�洢�ļ���ϸ��Ϣ*/
typedef struct Inode
{
	time_t time;
	unsigned int first;	/*�ļ���ʼ���*/
	unsigned int length;
	char type;	/* 0 Ϊ��ͨ�ļ���1 ΪĿ¼��-1��ʼ��ֵ*/
}AttributeNode;

typedef struct FCB
{
	IndexNode* index;
	Inode* inode;
}FCB;

#endif

