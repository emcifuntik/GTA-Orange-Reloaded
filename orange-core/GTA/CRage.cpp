#include "stdafx.h"

PedFactoryHook* PedFactoryHook::singleInstance = nullptr;
VehicleFactoryHook* VehicleFactoryHook::singleInstance = nullptr;
bool SyncTree::initialized = false;
GetSyncTree_ SyncTree::GetSyncTree;


namespace rageGlobals
{
	void SetPlayerColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
	{
		unsigned char * colorAddress = (unsigned char *)((ULONGLONG)CMemory((uintptr_t)GetModuleHandle(NULL) + 0x1E5C90).getOffset(2) + 4);
		for (int i = 0; i < 4; ++i)
		{
			(*colorAddress++) = b;
			(*colorAddress++) = g;
			(*colorAddress++) = r;
			(*colorAddress++) = a;
		}
	}
};

namespace GTA
{
	std::string CTask::GetTree(CTask *task, int n)
	{
		if (!n)
		{
			task = this;
			return VTasks::Get()->GetTaskName(this->GetID()) + GetTree(task->SubTask, n + 1);
		}
		else
		{
			std::string res("\n");
			if (!task)
				return res;
			for (int i = 0; i < n; ++i)
				res += "-";
			res += " ";
			res += VTasks::Get()->GetTaskName(task->GetID());
			return res + GetTree(task->SubTask, n + 1);
		}
	}

	CViewportGame *CViewportGame::Get()
	{
		return *(CViewportGame**)(CMemory((uintptr_t)GetModuleHandle(NULL) + 0xA27578).getOffset());
	}

};

GTA::CTask * CTaskTree::GetTaskByID(unsigned int taskID)
{
	for (GTA::CTask *task = GetTask(); task; task = task->SubTask)
		if (task->GetID() == taskID)
			return task;
	return nullptr;
}

CPed * CPed::GetFromScriptID(int Handle)
{
	typedef CPed*(*GetCEntity)(int);
	return (CPed*)((GetCEntity)CMemory((uintptr_t)GetModuleHandle(NULL) + 0x15013C)())(Handle);
}

CWorld *CWorld::Get()
{
	return *((CWorld**)CMemory((uintptr_t)GetModuleHandle(NULL) + 0x89E04D).getOffset());
}

CVehicleFactory* CVehicleFactory::Get()
{
	return (CVehicleFactory*)CMemory((uintptr_t)GetModuleHandle(NULL) + 0xE43BF4).getOffset();
}