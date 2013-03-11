#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include "hardware/SpaceManager.h"
#include <string>
using namespace std;

class DiskManager
{

public:
	static bool format(string,unsigned int);
	static bool open(string);
	static bool flush();
	static void print();

};


bool DiskManager::format(string filename,unsigned int blockSize)
{
	return SpaceManager::getInstance()->format(filename,blockSize);
}

bool DiskManager::open(string filename)
{
	return SpaceManager::getInstance()->open(filename);
}
bool DiskManager::flush()
{
	return SpaceManager::getInstance()->flush();
}

void DiskManager::print()
{
	SpaceManager::getInstance()->printIndex();
}
#endif

