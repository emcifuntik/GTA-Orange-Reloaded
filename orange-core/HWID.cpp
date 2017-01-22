#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN  
#include <windows.h>      
#include <intrin.h>       
#include <iphlpapi.h>     
#include <iostream>
#include <locale>
#include "md5.h"

#pragma comment(lib, "Iphlpapi.lib")


// we just need this for purposes of unique machine id. So any one or two mac's is       
// fine. 
int hashMacAddress(PIP_ADAPTER_INFO info)
{
	int hash = 0;
	for (unsigned long int i = 0; i < info->AddressLength; i++)
	{
		hash += (info->Address[i] << ((i & 1) * 8));
	}
	return hash;
}

void getMacHash(int& mac1, int& mac2)
{
	IP_ADAPTER_INFO AdapterInfo[32];
	DWORD dwBufLen = sizeof(AdapterInfo);

	DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
	if (dwStatus != ERROR_SUCCESS)
		return; // no adapters.      

	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
	mac1 = hashMacAddress(pAdapterInfo);
	if (pAdapterInfo->Next)
		mac2 = hashMacAddress(pAdapterInfo->Next);

	// sort the mac addresses. We don't want to invalidate     
	// both macs if they just change order.           
	if (mac1 > mac2)
	{
		int tmp = mac2;
		mac2 = mac1;
		mac1 = tmp;
	}
}

int getVolumeHash()
{
	DWORD serialNum = 0;

	// Determine if this volume uses an NTFS file system.      
	GetVolumeInformation(L"c:\\", NULL, 0, &serialNum, NULL, NULL, NULL, 0);
	int hash = (int)((serialNum + (serialNum >> 16)) & 0xFFFF);

	return hash;
}

int getCpuHash()
{
	int cpuinfo[4] = { 0, 0, 0, 0 };
	__cpuid(cpuinfo, 0);
	int hash = 0;
	int* ptr = (int*)(&cpuinfo[0]);
	for (long int i = 0; i < 8; i++)
		hash += ptr[i];

	return hash;
}

const char* getMachineName()
{
	static char computerName[1024];
	DWORD size = 1024;
	GetComputerNameA(computerName, &size);
	return &(computerName[0]);
}

void toClipboard(HWND hwnd, const std::string &s) {
	OpenClipboard(hwnd);
	EmptyClipboard();
	HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, s.size() + 1);
	if (!hg) {
		CloseClipboard();
		return;
	}
	memcpy(GlobalLock(hg), s.c_str(), s.size() + 1);
	GlobalUnlock(hg);
	SetClipboardData(CF_TEXT, hg);
	CloseClipboard();
	GlobalFree(hg);
}

std::string getHWID()
{
	std::string str = getMachineName();
	char vol[16];
	_itoa_s(getVolumeHash(), vol, 16);
	str += vol;
	int mac1, mac2;
	getMacHash(mac1, mac2);
	char mac1s[16] = "";
	_itoa_s(mac1, mac1s, 16, 16);
	char mac2s[16] = "";
	_itoa_s(mac2, mac2s, 16, 16);
	str += mac1s;
	str += mac2s;
	return md5(md5(str));
}