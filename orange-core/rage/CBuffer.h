#pragma once
namespace rage {
	enum BufferFlag {
		BufferRead = 1,
		BufferWrite = 2,
		BufferLog = 4
	};
	class CBuffer
	{
	public:
		unsigned char *Data; //0x0000 
		DWORD Align; //0x0008 
		DWORD StartSize; //0x000C 
		DWORD FinalSize; //0x0010 
		DWORD Size; //0x0014 
		DWORD Unknown; //0x0018 
		unsigned char Flags; //0x001C 
		static void Allocate(CBuffer * buff);
		static void InitRead(CBuffer * buff, unsigned char * packed, unsigned int size, int flags);
		static void InitWrite(CBuffer * buff, unsigned char * packed, unsigned int size, int flags);
	}; //Size=0x001D
}
