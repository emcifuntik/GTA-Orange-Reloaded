#pragma once
namespace rage {
	class CDynamicEntitySyncTreeBase : public rage::CEntitySyncTreeBase
	{
	public:
		rage::netSyncDataNode dynamicEntityGameStateDataNode; //0x0CD8 
		rage::ICommonDataOperations dynamicEntityGameStateManager; //0x0D88 
	private:
		char pad_0x0DA0[0x80]; //0x0DA0

	}; //Size=0x0E20
}