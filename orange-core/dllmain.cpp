// dllmain.cpp: определяет точку входа для приложения DLL.
#include "stdafx.h"

std::string my_ostream::fname = "output.log";

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		char TOrangeFolder[MAX_PATH];
		DWORD folderNameLen = MAX_PATH;
		if (!Registry::Get_StringRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\Orange Team\\GTA Orange", "OrangeFolder", TOrangeFolder, folderNameLen))
		{
			TerminateProcess(GetCurrentProcess(), EXIT_SUCCESS);
			return true;
		}
		CGlobals::Get().orangePath = TOrangeFolder;

		my_ostream::SetLogFile(CGlobals::Get().orangePath + "/client.log");
		PreLoadPatches();
#ifdef _DEBUG
		AllocConsole();
		SetConsoleTitle(L"Grand Theft Auto: Orange");
		FILE * unused = NULL;
		freopen_s(&unused, "CONOUT$", "w", stdout);
		freopen_s(&unused, "CONOUT$", "w", stderr);
#endif
		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
#ifdef _DEBUG
		CloseConsole();
#endif
		break;
	}
	return TRUE;
}

