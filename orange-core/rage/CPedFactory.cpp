#include "stdafx.h"
namespace rage {
	rage::CPedFactory * CPedFactory::Get()
	{
		return *((CPedFactory**)CMemory((uintptr_t)GetModuleHandle(NULL) + 0x89E04D).getOffset());
	}
	rage::CPed * CPedFactory::Create(int pedType, unsigned int hash, Vector3 * position, int64_t heading, bool network, bool a6)
	{
		typedef rage::CPed*(*Create_)(int pedType, unsigned int hash, Vector3 * position, int64_t heading, bool network, bool a6);
		return Create_((uintptr_t)GetModuleHandle(NULL) + 0xC6DD40)(pedType, hash, position, heading, network, a6);
	}
}