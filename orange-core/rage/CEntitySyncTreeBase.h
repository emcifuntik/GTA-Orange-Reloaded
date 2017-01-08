#pragma once
namespace rage {
	class CEntitySyncTreeBase : public rage::CProjectSyncTree
	{
	public:
		rage::CProjectBaseSyncParentNode projectBaseSyncParentNode1; //0x0918 
		rage::CProjectBaseSyncParentNode projectBaseSyncParentNode2; //0x0978 
		rage::CProjectBaseSyncParentNode projectBaseSyncParentNode3; //0x09D8 
		rage::CProjectBaseSyncParentNode projectBaseSyncParentNode4; //0x0A38 
		rage::CProjectBaseSyncParentNode projectBaseSyncParentNode5; //0x0A98 
		rage::netSyncDataNode entityScriptInfoDataNode; //0x0AF8 
		rage::ICommonDataOperations entityScriptInfoManager; //0x0BA8 
		rage::CGameScriptObjInfo gameScriptObjInfo; //0x0BC0 

	}; //Size=0x0CD8
}