#include "stdafx.h"
#include "SpaceBlock.h"
#include <string>
#include <iostream>
using namespace std;

unsigned int SpaceBlock::size=0;	/* !±ØÐëµÄ */

SpaceBlock::SpaceBlock(unsigned int page, char *buffer)
{
	this->page=page;
	this->buffer=buffer;
	this->offset=0;
}

bool SpaceBlock::seekg(unsigned int value)
{
	if(value>size)	// to ?
		return false;
	this->offset=value;
	return true;
}
unsigned int SpaceBlock::tellg()
{
	return this->offset;
}

unsigned int SpaceBlock::getPage()
{
	return this->page;
}

unsigned int SpaceBlock::getSize()
{
	unsigned int size=SpaceBlock::size;
	return size;
}
void SpaceBlock::setSize(unsigned int size)
{
	SpaceBlock::size=size;
}

