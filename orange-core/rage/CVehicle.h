#pragma once
namespace rage {
	class CVehicle : public rage::CPhysical
	{
		char pad_0x0300[0x18]; //0x0300
		char pad_0x0318[0x13C]; //0x0318
	public:
		float fDamage1; //0x0454 
		float fDamage2; //0x0458 
		float fDamage3; //0x045C 
		float fDamage4; //0x0460 
		float fDamage5; //0x0464 
		float fDamage6; //0x0468 
		float fDamage7; //0x046C 
		float fDamage8; //0x0470 
		float fDamage9; //0x0474 
		float fDamage10; //0x0478 
		float fDamage11; //0x047C 
		float fDamage12; //0x0480 
		float fDamage13; //0x0484 
		float fDamage14; //0x0488 
		float fDamage15; //0x048C 
		float fDamage16; //0x0490 
		float fDamage17; //0x0494 
		float fDamage18; //0x0498 
		float fDamage19; //0x049C 
		float fDamage20; //0x04A0 
		float fDamage21; //0x04A4 
		float fDamage22; //0x04A8 
		float fDamage23; //0x04AC 
		float fDamage24; //0x04B0 
		float fDamage25; //0x04B4 
	private:
		char pad_0x04B8[0x278]; //0x04B8
		CVector3 N00000637; //0x0730 
		char pad_0x073C[0x4]; //0x073C
		CVector3 N00000639; //0x0740 
		char pad_0x074C[0xA8]; //0x074C
	public:
		float fEngineRPM; //0x07F4 
		float fEngineRPM_2; //0x07F8 
	private:
		char pad_0x07FC[0x4]; //0x07FC
	public:
		float fEngineAccelerating; //0x0800 
		float fAccelerating; //0x0804 
		float fStartedAccelerating; //0x0808 
		float fBurnoutTime; //0x080C 
	private:
		char pad_0x0810[0x70]; //0x0810
		char pad_0x0884[0x4]; //0x0884
	public:
		rage::audCarAudioEntity* vehAudio; //0x0888 
	private:
		char pad_0x0890[0x34]; //0x0890
	public:
		float fSteering2; //0x08C4 
	private:
		char pad_0x08C8[0x4]; //0x08C8
	public:
		float fSteering; //0x08CC 
	private:
		char pad_0x08D0[0x4]; //0x08D0
	public:
		float fAccelerateValue3; //0x08D4 
		float fBurnoutValue; //0x08D8 
		DWORD dwHandBrake; //0x08DC 
	private:
		char pad_0x08E0[0xE8]; //0x08E0
	public:
		float fSpeed; //0x09C8 
		char pad_0x09CC[0xF4]; //0x09CC
	public:
		rage::CVehicleIntelligence* intelligence; //0x0AC0 
	private:
		char pad_0x0AC8[0x7B0]; //0x0AC8

	}; //Size=0x1278
}
