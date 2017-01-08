#pragma once
namespace rage {
	class CCustomShaderEffectBaseType
	{
	public:
		virtual ~CCustomShaderEffectBaseType();
		char pad_0x0008[0x98]; //0x0000
		float fUnknown50; //0x00A0 
		rage::bgraColor bgraPrimary; //0x00A4 
		rage::bgraColor bgraSecondary; //0x00A8 
		rage::bgraColor bgraPearl; //0x00AC 
		rage::bgraColor bgraUnknown1; //0x00B0 
		rage::bgraColor bgraUnknown2; //0x00B4 
		rage::bgraColor bgraUnknown3; //0x00B8 
		rage::bgraColor bgraUnknown4; //0x00BC 
		char pad_0x00C0[0x8]; //0x00C0

	}; //Size=0x00C8
};