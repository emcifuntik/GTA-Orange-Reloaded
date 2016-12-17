#pragma once

class CMemory
{
	void *address;
	static bool memoryCompare(const BYTE *data, const BYTE *pattern, size_t length);
	static bool memoryCompare(const BYTE *data, const BYTE *pattern, const char *mask);
public:
	template<typename T, T Value>
	static T Return()
	{
		return Value;
	}

	CMemory(UINT64 address);
	~CMemory();

	void put(const char *value);
	template <typename T> void put(T value);
	template <typename T> void put(void *address, T value);
	template <typename T> void jump(T func);
	template <typename T> void call(T func);
	void farJmp(LPVOID func);
	void retn();
	template <typename T> T* get(int offset);
	template <typename T> T get_call();

	LPVOID getOffset(int offset = 3)
	{
		long* ptr = (long*)((intptr_t)address + offset);
		return (LPVOID)(*ptr + ((intptr_t)address + offset + sizeof(long)));
	}

	DWORD getFunc()
	{
		return (DWORD)((uintptr_t)address);
	}

	CMemory& operator+(uintptr_t offset)
	{
		return *(new CMemory((uintptr_t)(this->address) + offset));
	}

	CMemory& operator-(uintptr_t offset)
	{
		return *(new CMemory((uintptr_t)(this->address) - offset));
	}

	uintptr_t operator()()
	{
		return (uintptr_t)this->address;
	}

	void nop(size_t length);

	static CMemory& Find(const char* pattern);
	static bool Check(std::string search, std::string mask, UINT64 offset);
};

template<typename T>
inline void CMemory::put(T value)
{
	DWORD dwProtectOld, back;
	VirtualProtect((LPVOID)address, sizeof(T), PAGE_EXECUTE_READWRITE, &dwProtectOld);
	memcpy(address, &value, sizeof(T));
	VirtualProtect((LPVOID)address, sizeof(T), dwProtectOld, &back);
	address = (LPVOID)((uint64_t)address + sizeof(T));
}

template<typename T>
inline void CMemory::put(void * address, T value)
{
	DWORD dwProtectOld, back;
	VirtualProtect((LPVOID)address, sizeof(T), PAGE_EXECUTE_READWRITE, &dwProtectOld);
	memcpy(address, &value, sizeof(T));
	VirtualProtect((LPVOID)address, sizeof(T), dwProtectOld, &back);
}

template<typename T>
inline void CMemory::jump(T func)
{
	put(address, (uint8_t)0xE9);
	put((void*)((size_t*)address + 1), (intptr_t)func - (intptr_t)baseDiff - 5);
}

template<typename T>
inline void CMemory::call(T func)
{
	put(address, (uint8_t)0xE8);
	put((void*)((uintptr_t)address + 1), DWORD((intptr_t)func - (intptr_t)baseDiff - 5));
}

inline void CMemory::retn()
{
	put(address, (uint8_t)0xC3);
}

template<typename T>
inline T* CMemory::get(int offset)
{
	char* ptr = reinterpret_cast<char*>(address);
	return reinterpret_cast<T*>(ptr + offset);
}

template<typename T>
inline T CMemory::get_call()
{
	intptr_t target = *(long*)(intptr_t(address) + 1);
	target += (intptr_t(address) + 5);
	return (T)target;
}