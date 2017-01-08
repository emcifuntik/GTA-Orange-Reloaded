#include "stdafx.h"
namespace rage {
	rage::CPedFactory * CPedFactory::Get()
	{
		return *((CPedFactory**)CMemory((uintptr_t)GetModuleHandle(NULL) + 0x89E04D).getOffset());
	}
}