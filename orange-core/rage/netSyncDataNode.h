#pragma once
namespace rage {
	class netSyncDataNode
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

		char pad_0x0008[0xA0]; //0x0008
		rage::ICommonDataOperations* manager; //0x00A8 

	}; //Size=0x00B0
}