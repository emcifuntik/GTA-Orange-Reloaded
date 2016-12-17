#include "stdafx.h"

#define _MEM_CACHE_FILE "memcache.dat"

std::map<UINT, DWORD> CMemoryCache::memoryHooks;
CMemoryCache * CMemoryCache::singleInstance;

CMemoryCache::CMemoryCache()
{
	std::ifstream bin(_MEM_CACHE_FILE, std::ifstream::binary);
	while (!bin.eof())
	{
		UINT hash;
		DWORD address;
		bin.read((char*)&hash, sizeof(UINT));
		bin.read((char*)&address, sizeof(DWORD));
		memoryHooks.insert(std::pair<UINT, DWORD>(hash, address));
	}
	bin.close();
}


bool CMemoryCache::TryGetOffset(std::string pattern, DWORD & offset)
{
	UINT hash = Utils::Hash(pattern.c_str());
	if (memoryHooks.find(hash) != memoryHooks.end())
	{
		offset = memoryHooks[hash];
		return true;
	}
	return false;
}

void CMemoryCache::AddOffset(std::string pattern, DWORD offset)
{
	UINT hash = Utils::Hash(pattern.c_str());
	if (memoryHooks.find(hash) == memoryHooks.end())
	{
		memoryHooks.insert(std::pair<UINT, DWORD>(hash, offset));
		Save();
	}
}

void CMemoryCache::Save()
{
	std::ofstream bin(_MEM_CACHE_FILE, std::ofstream::binary);
	for each (auto cache in memoryHooks)
	{
		bin.write((char*)&cache.first, sizeof(UINT));
		bin.write((char*)&cache.second, sizeof(DWORD));
	}
	bin.close();
}

CMemoryCache::~CMemoryCache()
{
}