#pragma once
namespace rage {
	class fiDeviceRelative {
	public:
		static bool MountFolder(const char * folder, const char * mountTarget, unsigned char * rageDevice = nullptr);
	};
}