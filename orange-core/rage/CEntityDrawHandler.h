#pragma once
namespace rage {
	class CEntityDrawHandler
	{
		char pad_0x0000[0x8]; //0x0000
	public:
		rage::fragDrawable* entityFragDrawable; //0x0008 
	private:
		char pad_0x0010[0x10]; //0x0010
	public:
		rage::CCustomShaderEffectBaseType* shader; //0x0020 
	private:
		char pad_0x0028[0x60]; //0x0028

	}; //Size=0x0088
}