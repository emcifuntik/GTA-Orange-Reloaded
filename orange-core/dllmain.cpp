// dllmain.cpp: ���������� ����� ����� ��� ���������� DLL.
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
		CGlobals::Get().dllModule = hModule;
		CGlobals::Get().orangePath = GetModuleDir();

		//std::stringstream path;
		//path << CGlobals::Get().orangePath << "\\bin;" << std::getenv("PATH");

		//_putenv_s("PATH", path.str().c_str());

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

