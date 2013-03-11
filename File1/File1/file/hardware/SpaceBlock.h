#ifndef SPACEBLOCK_H
#define SPACEBLOCK_H

#include <string>
using namespace std;
class SpaceBlock
{
public :
	//构造函数，n：盘块号，a：该盘块号的起始位置
	SpaceBlock(unsigned int, char *);	// page, buffer

	//读n个字符到a指向的字符串中，并将偏移量加上n，返回读成功的个数
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

	//写入a指向的字符串前n个字符，并将偏移量加上n，返回写成功的个数
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

	//读出一个类型为type的数据，并将偏移量加上sizeof(type)，返回读成功的个数
	template <class type>
	unsigned int read(type *value)
	{
		unsigned int c=sizeof(type);
		if(offset+c>size)return 0;
		char *a=reinterpret_cast<char *> (value);
		
		//value=reinterpret_cast<type *> (a);		
		return read(a,c);
	}

	//写入一个类型为type的数据，并将偏移量加上sizeof(type)，返回写成功的个数
	template <class type>
	unsigned int write(type *value)
	{
		unsigned int c=sizeof(type);
		if(offset+c>size)return 0;
		char *a=reinterpret_cast<char *> (value);
		return write(a,c);
	}	

	unsigned int getPage();
	bool seekg(unsigned int);	//设置偏移量指针
	unsigned int tellg();	//查询偏移量指针

	static unsigned int getSize();	//获取盘块大小
	static void setSize(unsigned int);	//设置盘块大小，设定后不再改变
	
protected :
	unsigned int offset;
	unsigned int page;
	char *buffer;

	static unsigned int size;
	
};


#endif