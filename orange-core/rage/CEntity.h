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

		char pad_0x0008[0x4]; //0x0008
		DWORD dwUnknown73; //0x000C 
		char pad_0x0010[0x8]; //0x0010
		DWORD dwEntityFlags; //0x0018 
		char pad_0x001C[0x4]; //0x001C
		rage::CBaseModelInfo* modelInfo; //0x0020 
		DWORD dwUnknown72; //0x0028 
		DWORD dwUnknown70; //0x002C 
		rage::phInstGta* instGta; //0x0030 
		DWORD dwUnknown71; //0x0038 
		char pad_0x003C[0x4]; //0x003C
		DWORD dwUnknown74; //0x0040 
		DWORD dwUnknown75; //0x0044 
		rage::CEntityDrawHandler* drawHandler; //0x0048 
		char pad_0x0050[0x10]; //0x0050
		float fHeading; //0x0060 
		float fHeadingAlt; //0x0064 
		float fUnknown40; //0x0068 
		float fUnknown41; //0x006C 
		float fUnknown42; //0x0070 
		float fUnknown43; //0x0074 
		float fUnknown44; //0x0078 
		float fUnknown45; //0x007C 
		float fUnknown46; //0x0080 
		float fUnknown47; //0x0084 
		float fUnknown48; //0x0088 
		float fUnknown49; //0x008C 
		CVector3 vecPositionEntity; //0x0090 
		char pad_0x009C[0x34]; //0x009C

	}; //Size=0x00D0
}
