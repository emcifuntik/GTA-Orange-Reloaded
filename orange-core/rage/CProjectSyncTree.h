#pragma once
namespace rage {
	class CProjectSyncTree : public rage::netSyncTree
	{
		char pad_0x04B8[0x10]; //0x04B8
	public:
		rage::CProjectBaseSyncParentNode projectBaseSyncParentNode1; //0x04C8 
		rage::CProjectBaseSyncParentNode projectBaseSyncParentNode2; //0x0528 
		rage::CProjectBaseSyncParentNode projectBaseSyncParentNode3; //0x0588 
		rage::netSyncDataNode migrationDataNode; //0x05E8 
		rage::ICommonDataOperations migrationManager; //0x0698 
	private:
		char pad_0x06B0[0x8]; //0x06B0
	public:
		rage::netSyncDataNode globalFlagsDataNode; //0x06B8 
		rage::ICommonDataOperations globalFlagsManager; //0x0768 
		rage::netSyncDataNode sectorDataNode; //0x0780 
		rage::ICommonDataOperations sectorManager; //0x0830 
		rage::netSyncDataNode sectorPositionDataNode; //0x0848 
		rage::ICommonDataOperations sectorPositionManager; //0x08F8 
	private:
		char pad_0x0910[0x8]; //0x0910

	}; //Size=0x0918
}