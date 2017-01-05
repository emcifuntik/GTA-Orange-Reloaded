#pragma once
namespace rage {
	class CEntity
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

		char pad_0x0008[0x10]; //0x0008
		DWORD dwEntityFlags; //0x0018 
		char pad_0x001C[0x4]; //0x001C
		rage::CBaseModelInfo* modelInfo; //0x0020 
		char pad_0x0028[0x8]; //0x0028
		rage::phInstGta* instGta; //0x0030 
		char pad_0x0038[0x28]; //0x0038
		float fHeading; //0x0060 
		float fHeadingAlt; //0x0064 
		char pad_0x0068[0x28]; //0x0068
		CVector3 vecPositionEntity; //0x0090 
		char pad_0x009C[0x34]; //0x009C

	}; //Size=0x00D0
}
