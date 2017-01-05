#pragma once
namespace rage {
	class CPhysical : public rage::CDynamicEntity
	{
	public:
		rage::CEntity* entityPtrUnknown1; //0x00F8 
		DWORD dwUnknown1; //0x0100 
		char pad_0x0104[0xC]; //0x0104
		CVector3 vecPositionPhysical1; //0x0110 
		char pad_0x011C[0x4]; //0x011C
		CVector3 vecPositionPhysical2; //0x0120 
		char pad_0x012C[0x48]; //0x012C
		DWORD dwUnknown3; //0x0174 
		char pad_0x0178[0x10]; //0x0178
		DWORD dwPhysicalFlags1; //0x0188 
		char pad_0x018C[0xE4]; //0x018C
		float fUnknown1; //0x0270 
		char pad_0x0274[0x4]; //0x0274
		DWORD dwPhysicalFlags2; //0x0278 
		char pad_0x027C[0x4]; //0x027C
		float fHealth; //0x0280 
		char pad_0x0284[0x1C]; //0x0284
		float fMaxHealth; //0x02A0 
		char pad_0x02A4[0x4C]; //0x02A4
		DWORD dwUnknown4; //0x02F0 
		DWORD dwUnknown5; //0x02F4 
		DWORD dwUnknown6; //0x02F8 
		char pad_0x02FC[0x4]; //0x02FC

	}; //Size=0x0300
}