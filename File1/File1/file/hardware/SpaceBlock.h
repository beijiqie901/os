#ifndef SPACEBLOCK_H
#define SPACEBLOCK_H

#include <string>
using namespace std;
class SpaceBlock
{
public :
	//���캯����n���̿�ţ�a�����̿�ŵ���ʼλ��
	SpaceBlock(unsigned int, char *);	// page, buffer

	//��n���ַ���aָ����ַ����У�����ƫ��������n�����ض��ɹ��ĸ���
	unsigned int read(char * a,unsigned int n)
	{
		unsigned int min=offset+n<size?n:size-offset;
		for(unsigned int i=0;i<min;i++)
		{
			*(a+i)=this->buffer[this->offset+i];
		}
		this->offset=this->offset+min;
		return min;
	}

	//д��aָ����ַ���ǰn���ַ�������ƫ��������n������д�ɹ��ĸ���
	unsigned int write(char * a,unsigned int n)
	{
		unsigned int min=offset+n<size?n:size-offset;
		for(unsigned int i=0;i<min;i++)
		{
			this->buffer[this->offset+i]=*(a+i);
		}
		this->offset=this->offset+min;
		return min;
	}

	//����һ������Ϊtype�����ݣ�����ƫ��������sizeof(type)�����ض��ɹ��ĸ���
	template <class type>
	unsigned int read(type *value)
	{
		unsigned int c=sizeof(type);
		if(offset+c>size)return 0;
		char *a=reinterpret_cast<char *> (value);
		
		//value=reinterpret_cast<type *> (a);		
		return read(a,c);
	}

	//д��һ������Ϊtype�����ݣ�����ƫ��������sizeof(type)������д�ɹ��ĸ���
	template <class type>
	unsigned int write(type *value)
	{
		unsigned int c=sizeof(type);
		if(offset+c>size)return 0;
		char *a=reinterpret_cast<char *> (value);
		return write(a,c);
	}	

	unsigned int getPage();
	bool seekg(unsigned int);	//����ƫ����ָ��
	unsigned int tellg();	//��ѯƫ����ָ��

	static unsigned int getSize();	//��ȡ�̿��С
	static void setSize(unsigned int);	//�����̿��С���趨���ٸı�
	
protected :
	unsigned int offset;
	unsigned int page;
	char *buffer;

	static unsigned int size;
	
};


#endif