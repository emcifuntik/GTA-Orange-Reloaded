#pragma once
namespace rage {
	class CPlayerPedTargeting
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

		rage::CPed* pedOwner; //0x0008 
		float fAimTime; //0x0010 
		DWORD dwAimState; //0x0014 
		DWORD dwCurrentWeapon; //0x0018 
		char pad_0x001C[0x44]; //0x001C
		CVector3 vecAimPosition; //0x0060 
		float fAimLeft; //0x006C 
		char pad_0x0070[0x98]; //0x0070
		DWORD dwTotalAimTime; //0x0108 
		char pad_0x010C[0x4]; //0x010C
		rage::CEntity* aimAtEntity; //0x0110 
		DWORD dwLastAimTime; //0x0118 
		char pad_0x011C[0x34]; //0x011C
		int8_t bAimAtEntity; //0x0150 
		char pad_0x0151[0x2F]; //0x0151

	}; //Size=0x0180
}
