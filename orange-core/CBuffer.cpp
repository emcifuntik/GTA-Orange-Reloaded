#include "stdafx.h"
namespace rage {
	void CBuffer::Allocate(CBuffer * buff)
	{
		typedef void(*InitBuffer)(CBuffer*);
		InitBuffer((uintptr_t)GetModuleHandle(NULL) + 0x11E7920)(buff);
	}
	void CBuffer::InitRead(CBuffer * buff, unsigned char * packed, unsigned int size, int flags)
	{
		typedef void(*InitReadBuffer)(CBuffer*, unsigned char*, int, int);
		((InitReadBuffer)CMemory((uintptr_t)GetModuleHandle(NULL) + 0x11EBCA8)())(buff, packed, size, 0);
	}
	void CBuffer::InitWrite(CBuffer * buff, unsigned char * packed, unsigned int size, int flags)
	{
		typedef void(*InitWriteBuffer)(CBuffer*, unsigned char*, int, int);
		((InitWriteBuffer)CMemory((uintptr_t)GetModuleHandle(NULL) + 0x11EBCEC)())(buff, packed, size, 0);
	}
}