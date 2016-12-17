#pragma once
class CMemoryCache
{
	static CMemoryCache * singleInstance;
	static std::map<UINT, DWORD> memoryHooks;
	CMemoryCache();
public:
	static CMemoryCache& Get()
	{
		if (!singleInstance)
			singleInstance = new CMemoryCache();
		return *singleInstance;
	}
	bool TryGetOffset(std::string pattern, DWORD& offset);
	void AddOffset(std::string pattern, DWORD offset);
	void Save();
	~CMemoryCache();
};
