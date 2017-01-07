#pragma once
namespace rage {
	class CTask
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
		rage::CEntity* Owner; //0x0010 
		rage::CTask* Parent; //0x0018 
		rage::CTask* Child; //0x0020 
		rage::CTaskTree* NextChild; //0x0028 
		int32_t iUnknown1; //0x0030 
		int32_t iUnknown2; //0x0034 
		float timeFromBegin; //0x0038 
		float totalTime; //0x003C 
		char pad_0x0040[0x50]; //0x0040

	}; //Size=0x0090

}