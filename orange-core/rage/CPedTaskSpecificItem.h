#pragma once
namespace rage {
	class CPedTaskSpecificItem : public rage::netSyncDataNode
	{
	public:
		rage::ICommonDataOperations pedTaskSpecificManager; //0x00B0 
		char pad_0x00C8[0x200]; //0x00C8

	}; //Size=0x02C8
}