#pragma once
namespace rage {
	class CPedSyncTreeBase : public rage::CDynamicEntitySyncTreeBase
	{
	public:
		rage::CProjectBaseSyncParentNode projectBaseSyncParentNode1; //0x0E20 
		rage::netSyncDataNode physicalMigrationDataNode; //0x0E80 
		rage::ICommonDataOperations physicalMigrationManager; //0x0F30 
		rage::netSyncDataNode physicalScriptMigrationDataNode; //0x0F48 
		rage::ICommonDataOperations physicalScriptMigrationManager; //0x0FF8 
	private:
		char pad_0x1010[0x8]; //0x1010
	public:
		rage::netSyncDataNode physicalGameStateDataNode; //0x1018 
		rage::ICommonDataOperations physicalGameStateManager; //0x10C8 
	private:
		char pad_0x10E0[0x8]; //0x10E0
	public:
		rage::netSyncDataNode pedOrientationDataNode; //0x10E8 
		rage::ICommonDataOperations pedOrientationManager; //0x1198 
		rage::netSyncDataNode pedMovementDataNode; //0x11B0 
		rage::ICommonDataOperations pedMovementManager; //0x1260 
	private:
		char pad_0x1278[0x8]; //0x1278
	public:
		rage::netSyncDataNode pedAttachDataNode; //0x1280 
		rage::ICommonDataOperations pedAttachManager; //0x1330 
	private:
		char pad_0x1348[0x38]; //0x1348
	public:
		rage::netSyncDataNode pedMovementGroupDataNode; //0x1380 
		rage::ICommonDataOperations pedMovementGroupManager; //0x1430 
	private:
		char pad_0x1448[0x38]; //0x1448
	public:
		rage::netSyncDataNode pedAIDataNode; //0x1480 
		rage::ICommonDataOperations pedAIManager; //0x1530 
		rage::netSyncDataNode pedGameStateDataNode; //0x1548 
		rage::ICommonDataOperations pedGameStateManager; //0x15F8 
	private:
		char pad_0x1610[0x98]; //0x1610
	public:
		rage::netSyncDataNode pedComponentReservationDataNode; //0x16A8 
		rage::ICommonDataOperations pedComponentReservationManager; //0x1758 
	private:
		char pad_0x1770[0x40]; //0x1770
	public:
		rage::netSyncDataNode pedHealthDataNode; //0x17B0 
		rage::ICommonDataOperations pedHealthManager; //0x1860 
	private:
		char pad_0x1878[0x18]; //0x1878
	public:
		rage::netSyncDataNode pedTaskTreeDataNode; //0x1890 
		rage::ICommonDataOperations pedTaskTreeManager; //0x1940 
	private:
		char pad_0x1958[0xA8]; //0x1958
	public:
		rage::CPedTaskSpecificItem pedTaskSpecificItem1;
		rage::CPedTaskSpecificItem pedTaskSpecificItem2;
		rage::CPedTaskSpecificItem pedTaskSpecificItem3;
		rage::CPedTaskSpecificItem pedTaskSpecificItem4;
		rage::CPedTaskSpecificItem pedTaskSpecificItem5;
		rage::CPedTaskSpecificItem pedTaskSpecificItem6;
		rage::CPedTaskSpecificItem pedTaskSpecificItem7;
		rage::CPedTaskSpecificItem pedTaskSpecificItem8;

	}; //Size=0x3040
}