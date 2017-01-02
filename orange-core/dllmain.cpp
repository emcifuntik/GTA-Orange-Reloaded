// dllmain.cpp: определяет точку входа для приложения DLL.
#include "stdafx.h"

std::string my_ostream::fname = "output.log";

std::string GetModuleDir()
{
	HMODULE hModule;
	char    cPath[MAX_PATH] = { 0 };
	GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCSTR)GetModuleDir, &hModule);

	GetModuleFileNameA(hModule, cPath, MAX_PATH);
	std::string path = cPath;
	return path.substr(0, path.find_last_of("\\/"));
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		/*char TOrangeFolder[MAX_PATH];
		DWORD folderNameLen = MAX_PATH;
		if (!Registry::Get_StringRegistryValue(HKEY_CURRENT_USER, "SOFTWARE\\GTA Orange Team\\GTA Orange", "OrangeFolder", TOrangeFolder, folderNameLen))
		{
			TerminateProcess(GetCurrentProcess(), EXIT_SUCCESS);
			return true;
		}*/
		CGlobals::Get().orangePath = GetModuleDir();

		my_ostream::SetLogFile(CGlobals::Get().orangePath + "/client.log");
		PreLoadPatches();
		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		FreeConsole();
		break;
	}
	return TRUE;
}

