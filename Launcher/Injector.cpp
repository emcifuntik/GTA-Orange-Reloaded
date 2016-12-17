#include "stdafx.h"

Injector * Injector::instance = nullptr;



Injector::Injector()
{
}

void Injector::Run(std::wstring folder, std::wstring pePath)
{
	TCHAR Params[] = L"";
	STARTUPINFO siStartupInfo;
	PROCESS_INFORMATION piProcessInfo;
	memset(&siStartupInfo, 0, sizeof(siStartupInfo));
	memset(&piProcessInfo, 0, sizeof(piProcessInfo));
	siStartupInfo.cb = sizeof(siStartupInfo);
	if (!CreateProcess(pePath.c_str(), Params, NULL, NULL, true, CREATE_SUSPENDED, NULL, folder.c_str(), &siStartupInfo, &piProcessInfo))
		throw new std::exception("Can't start executable");
	ResumeThread(piProcessInfo.hThread);
}

void Injector::RunSteam()
{
	ShellExecute(NULL, NULL, L"steam://run/271590", NULL, NULL, SW_SHOW);
}

void Injector::InjectAll(bool waitForUnpack)
{
	WaitUntilGameStarts();
	Sleep(100);
	int pid = FindProcess(PROCESS_NAME);
	if(waitForUnpack)
		WaitForUnpackFinished(pid);
	for each (std::string lib in libs)
	{
		if (!Inject(pid, lib.c_str()))
		{
			MessageBox(NULL, L"Not injected", L"Alert", MB_OK);
			return;
		}
	}
	Injected = true;
}

void Injector::PushLibrary(std::string path)
{
	if (Injected == true)
		throw new std::exception("Libraries already injected");
	if (!Utils::FileExist(Utils::MultibyteToUnicode(path)))
		throw new std::exception("Library doesn't exist");
	libs.push_back(path);
}

int Injector::FindProcess(std::wstring procName)
{
	HANDLE hSnap = INVALID_HANDLE_VALUE, hProcess = INVALID_HANDLE_VALUE;
	PROCESSENTRY32 ProcessStruct;
	ProcessStruct.dwSize = sizeof(PROCESSENTRY32);
	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
		return -1;
	if (Process32First(hSnap, &ProcessStruct) == FALSE)
		return -1;
	do {
		if (wcscmp(ProcessStruct.szExeFile, procName.c_str()) == 0) {
			CloseHandle(hSnap);
			return ProcessStruct.th32ProcessID;
		}
	} while (Process32Next(hSnap, &ProcessStruct));
	CloseHandle(hSnap);
	return -1;
}

GameVersion Injector::GetGameVersion()
{
	return GameVersion();
}

void Injector::WaitUntilGameStarts()
{
	while (FindProcess(L"GTA5.exe") == -1);
}

bool Injector::Inject(int processId, std::string dllName)
{
	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, false, processId);
	if (!process)
		return false;
	LPVOID LoadLibraryA_ = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");
	LPVOID LoadComp = VirtualAllocEx(process, NULL, dllName.length(), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	WriteProcessMemory(process, LoadComp, dllName.c_str(), dllName.length(), NULL);
	HANDLE injectThread = CreateRemoteThread(process, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryA_, LoadComp, 0, NULL);
	WaitForSingleObject(injectThread, INFINITE);
	VirtualFreeEx(process, LoadComp, dllName.length(), MEM_RELEASE);
	CloseHandle(injectThread);
	CloseHandle(process);
	return true;
}

void Injector::WaitForUnpackFinished(int pid)
{
	HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);

	MODULEENTRY32 ModEnt;
	ModEnt.dwSize = sizeof(MODULEENTRY32);

	HMODULE hMod = NULL;
	HANDLE Snapshot1 = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);

	do {
		Module32First(Snapshot1, &ModEnt);
		do {
			if (wcscmp(PROCESS_NAME, ModEnt.szModule) == 0)
			{
				hMod = ModEnt.hModule;
				break;
			}
		} while (Module32Next(Snapshot1, &ModEnt));
	} while (hMod == NULL);

	unsigned char buff[10];
	ReadProcessMemory(process, (LPVOID)((uint64_t)hMod + 0x1000), buff, 10, NULL);
	for (;;)
	{
		Sleep(1);
		unsigned char newBuff[10];
		ReadProcessMemory(process, (LPVOID)((uint64_t)hMod + 0x1000), newBuff, 10, NULL);
		for (int i = 0; i < 10; ++i)
		{
			if (buff[i] != newBuff[i])
			{
				CloseHandle(process);
				return;
			}
		}
	}
}


Injector::~Injector()
{
}
