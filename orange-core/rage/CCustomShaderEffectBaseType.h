#pragma once
namespace {
	class CCustomShaderEffectBaseType
	{
	public:
		char pad_0x0000[0xA0]; //0x0000
		float fUnknown50; //0x00A0 
		DWORD bgraPrimary; //0x00A4 
		DWORD bgraSecondary; //0x00A8 
		DWORD bgraPearl; //0x00AC 
		DWORD bgraUnknown1; //0x00B0 
		DWORD bgraUnknown2; //0x00B4 
		DWORD bgraUnknown3; //0x00B8 
		DWORD bgraUnknown4; //0x00BC 
		char pad_0x00C0[0x8]; //0x00C0

	}; //Size=0x00C8
}