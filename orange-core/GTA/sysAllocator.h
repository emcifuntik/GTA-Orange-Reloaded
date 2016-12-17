#pragma once

namespace rage
{
	enum
	{
		HEAP_TASK = 0,
		HEAP_TASK_CLONE,
		HEAP_LAST
	};
	class sysMemAllocator
	{
		sysMemAllocator();
		int64_t *heaps = nullptr;
		static sysMemAllocator* singleInstance;
	public:
		void* (*_allocate)(int64_t allocatorPtr, int64_t size, int64_t align, int64_t suballocator);
		void* allocate(int64_t size, int64_t align, int heapNumber = HEAP_TASK, int64_t suballocator = 0);

		void (*_free)(int64_t allocatorPtr, void* address);
		void free(void* address, int heapNumber = HEAP_TASK);

		static sysMemAllocator* Get()
		{
			if (!singleInstance)
				singleInstance = new sysMemAllocator();
			return singleInstance;
		}
	};

	class sysUseAllocator
	{
	public:
		void* operator new(size_t size);
		inline void* operator new[](size_t size)
		{
			return sysUseAllocator::operator new(size);
		}
		void operator delete(void* memory);
		inline void operator delete[](void* memory)
		{
			return sysUseAllocator::operator delete(memory);
		}
	};
}