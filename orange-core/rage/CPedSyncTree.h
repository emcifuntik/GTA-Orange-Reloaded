#pragma once
namespace rage {
	class CPedSyncTree : public rage::CPedSyncTreeBase
	{
	public:
		rage::netSyncDataNode pedCreationDataNode; //0x3040 
		rage::ICommonDataOperations pedCreationManager; //0x30F0 
	private:
		char pad_0x3108[0x28]; //0x3108
	public:
		rage::netSyncDataNode pedScriptCreationDataNode; //0x3130 
		rage::ICommonDataOperations pedScriptCreationManager; //0x31E0 
	private:
		char pad_0x31F8[0x8]; //0x31F8
	public:
		rage::netSyncDataNode pedSectorPosMapNode; //0x3200 
		rage::ICommonDataOperations sectorPositionManager; //0x32B0 
	private:
		char pad_0x32C8[0x8]; //0x32C8
	public:
		rage::ICommonDataOperations pedSectorPosMapManager; //0x32D0 
	private:
		char pad_0x32E8[0x18]; //0x32E8
	public:
		rage::netSyncDataNode pedSectorPosNavMeshNode; //0x3300 
		rage::ICommonDataOperations pedSectorPosNavMeshManager; //0x33B0 
	private:
		char pad_0x33C8[0x8]; //0x33C8
	public:
		rage::netSyncDataNode physicalScriptGameStateDataNode; //0x33D0 
		rage::ICommonDataOperations physicalScriptGameStateManager; //0x3480 
	private:
		char pad_0x3498[0x28]; //0x3498
	public:
		rage::netSyncDataNode pedScriptGameStateDataNode; //0x34C0 
		rage::ICommonDataOperations pedScriptGameStateManager; //0x3570 
	private:
		char pad_0x3588[0x188]; //0x3588
	public:
		rage::netSyncDataNode pedAppearanceDataNode; //0x3710 
		rage::ICommonDataOperations pedAppearanceManager; //0x37C0 
	private:
		char pad_0x37D8[0x120]; //0x37D8
	public:
		rage::netSyncDataNode pedInventoryDataNode; //0x38F8 
		rage::ICommonDataOperations pedInventoryManager; //0x39A8 
	private:
		char pad_0x39C0[0x1290]; //0x39C0
	public:
		rage::netSyncDataNode pedTaskSequenceDataNode; //0x4C50 
		rage::ICommonDataOperations pedTaskSequenceManager; //0x4D00 

	}; //Size=0x4D18
}