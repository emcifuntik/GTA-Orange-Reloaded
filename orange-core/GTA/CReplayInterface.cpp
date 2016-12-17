#include "stdafx.h"
ReplayInterfaces* ReplayInterfaces::Get()
{
	return *((ReplayInterfaces**)(CMemory((uintptr_t)GetModuleHandle(NULL) + 0x1CB4).getOffset()));
}