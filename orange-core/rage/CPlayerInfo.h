#pragma once
namespace rage {
	class CPlayerInfo
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

		char pad_0x0008[0x74]; //0x0008
		char N00000646[16]; //0x007C 
		char pad_0x008C[0x58]; //0x008C
		float fSwimSpeed; //0x00E4 
		float fRunSpeed; //0x00E8 
		char pad_0x00EC[0xA4]; //0x00EC
		DWORD dwPlayerFlags; //0x0190 
		char pad_0x0194[0x2C]; //0x0194
		rage::ioConstantLightEffect* lightEffect; //0x01C0 
		char pad_0x01C8[0x10]; //0x01C8
		rage::fwWantedLightEffect* wantedLightEffect; //0x01D8 
		char pad_0x01E0[0x20]; //0x01E0
		rage::CPlayerPedTargeting playerPedTargeting; //0x0200 
		char pad_0x0380[0xBF0]; //0x0380

	}; //Size=0x0F70
}