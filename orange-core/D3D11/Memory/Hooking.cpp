#include "stdafx.h"

// =================================================================================
// Init
// =================================================================================
void Memory::Init()
{
	// Init MinHook
	if (MH_Initialize() != MH_OK)
	{
		printf("[Memory::Init] MH_Initialize failed!\n");
		return;
	}
}
void Memory::CleanUp()
{
	MH_Uninitialize();
}

// =================================================================================
// Hook Function
// =================================================================================
bool Memory::HookFunction(DWORD64 pAddress, void* pDetour, void** ppOriginal)
{
	// Create Hook
	int iResult = MH_CreateHook((void*)pAddress, pDetour, ppOriginal);
	if (iResult != MH_OK)
	{
		printf("[Memory::HookFunction] MH_CreateHook failed: %I64x [Error %i]\n", pAddress, iResult);
		MessageBoxA(0, "CreateHook fail", "Fail", 0);
		return false;
	}

	// Enable Hook
	iResult = MH_EnableHook((void*)pAddress);
	if (iResult != MH_OK)
	{
		printf("[Memory::HookFunction] MH_EnableHook failed: %I64x [Error %i]\n", pAddress, iResult);
		MessageBoxA(0, "EnableHook fail", "Fail", 0);
		return false;
	}

	// Success
	return true;
}
bool Memory::HookLibraryFunction(char* sLibrary, char* sName, void* pDetour, void** ppOriginal)
{
	// Module
	HMODULE hModule = GetModuleHandleA(sLibrary);
	if (hModule == NULL)
	{
		printf("[Memory::HookLibraryFunction] Module %s not (yet) loaded!\n", sLibrary);
		return false;
	}

	// Proc
	void* pProc = GetProcAddress(hModule, sName);
	if (pProc == NULL)
	{
		printf("[Memory::HookLibraryFunction] Module %s has no exported function %s!\n", sLibrary, sName);
		return false;
	}

	// Hook
	return HookFunction((DWORD64)pProc, pDetour, ppOriginal);
}