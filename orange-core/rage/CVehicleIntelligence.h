#pragma once
namespace rage {
	class CVehicleIntelligence
	{
	public:
		char pad_0x0000[0x7B0]; //0x0000
		rage::CVehicleTaskManager taskManager; //0x07B0 
		char pad_0x08E0[0x1E8]; //0x08E0

	}; //Size=0x0AC8
}