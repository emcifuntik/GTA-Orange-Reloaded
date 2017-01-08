#pragma once
namespace rage {
	class CGameScriptObjInfo
	{
	public:
		virtual void Function0(); //
		virtual void Function1(); //
		virtual void Function2(); //
		virtual void Function3(); //
		virtual void Function4(); //
		virtual void Function5(); //
		virtual void Function6(); //
		virtual void Function7(); //
		virtual void Function8(); //
		virtual void Function9(); //

		char pad_0x0008[0x8]; //0x0008
		rage::CGameScriptId gameScriptId; //0x0010 
		rage::netSyncDataNode entityScriptGameStateDataNode; //0x0050 
		rage::ICommonDataOperations entityScriptGameStateManager; //0x0100 

	}; //Size=0x0118
}