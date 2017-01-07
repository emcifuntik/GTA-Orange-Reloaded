#include "stdafx.h"

bool rage::fiDeviceRelative::MountFolder(const char * folder, const char * mountTarget, unsigned char * orangeDevice)
{
	typedef int64_t(*SetPath_)(uintptr_t, const char*, bool, uintptr_t);//49 8B F9 48 8B D9 4C 8B CA - 0x17
	typedef bool(*Mount_)(uintptr_t, const char*);

	if (!orangeDevice)
	{
		unsigned char *orangeDevice = new unsigned char[288];
		*(uintptr_t*)orangeDevice = ((uintptr_t)GetModuleHandle(NULL) + 0x1832988);
		*(uintptr_t*)(orangeDevice + 8) = 0;
	}
	

	int64_t res = SetPath_((uintptr_t)GetModuleHandle(NULL) + 0x11CB18C)((uintptr_t)orangeDevice, folder, 0, 0);
	if (Mount_((uintptr_t)GetModuleHandle(NULL) + 0x11CC6A8)((uintptr_t)orangeDevice, mountTarget))
	{
		log_debug << "Mounted \"" << folder << "\" in \"" << mountTarget << "\"" << std::endl;
		return true;
	}
	else
	{
		log_error << "Not mounted \"" << folder << "\" in \"" << mountTarget << "\"" << std::endl;
		return true;
	}
}
