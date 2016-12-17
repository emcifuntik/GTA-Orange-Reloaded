#include "stdafx.h"

namespace rage
{
	sysMemAllocator::sysMemAllocator()
	{
		heaps = new int64_t[HEAP_LAST];
		heaps[HEAP_TASK] = *(int64_t*)CMemory((uintptr_t)GetModuleHandle(NULL) + 0x4E05).getOffset();
		heaps[HEAP_TASK_CLONE] = *(int64_t*)CMemory((uintptr_t)GetModuleHandle(NULL) + 0x6369FA).getOffset();
	}
	sysMemAllocator* sysMemAllocator::singleInstance = nullptr;
	void* sysUseAllocator::operator new(size_t size)
	{
		return sysMemAllocator::Get()->allocate(size, 16, 0);
	}

	void sysUseAllocator::operator delete(void* memory)
	{
		sysMemAllocator::Get()->free(memory);
	}

	void* sysMemAllocator::allocate(int64_t size, int64_t align, int heapNumber, int64_t suballocator)
	{
		typedef void*(*__func)(int64_t, int64_t, int64_t, int64_t);
		return (__func)CMemory((uintptr_t)GetModuleHandle(NULL) + 0x11D7D6C)();
	}

	void sysMemAllocator::free(void* address, int heapNumber)
	{
		typedef void(*__func)(int64_t, void*);
		((__func)CMemory((uintptr_t)GetModuleHandle(NULL) + 0x11D7DDC)())(heaps[heapNumber], address);
	}
}

