#pragma once
namespace rage {
	class CEventGroupPed
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

		char pad_0x0008[0x90]; //0x0008
		rage::CPed* owner; //0x0098 

	}; //Size=0x00A0
}