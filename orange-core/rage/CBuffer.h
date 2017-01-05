#pragma once
namespace rage {
	enum BufferFlag {
		BufferRead = 1,
		BufferWrite = 2,
		BufferLog = 4
	};
	class Buffer
	{
	public:
		unsigned char *Data; //0x0000 
		DWORD Align; //0x0008 
		DWORD StartSize; //0x000C 
		DWORD FinalSize; //0x0010 
		DWORD Size; //0x0014 
		DWORD Unknown; //0x0018 
		unsigned char Flags; //0x001C 
		static void Allocate(Buffer * buff)
		{
			typedef void(*InitBuffer)(Buffer*);
			InitBuffer((uintptr_t)GetModuleHandle(NULL) + 0x11E7920)(buff);
		}
		static void InitRead(Buffer * buff, unsigned char * packed, unsigned int size, int flags)
		{
			typedef void(*InitReadBuffer)(Buffer*, unsigned char*, int, int);
			((InitReadBuffer)CMemory((uintptr_t)GetModuleHandle(NULL) + 0x11EBCA8)())(buff, packed, size, 0);
		}
		static void InitWrite(Buffer * buff, unsigned char * packed, unsigned int size, int flags)
		{
			typedef void(*InitWriteBuffer)(Buffer*, unsigned char*, int, int);
			((InitWriteBuffer)CMemory((uintptr_t)GetModuleHandle(NULL) + 0x11EBCEC)())(buff, packed, size, 0);
		}
	}; //Size=0x001D
}
