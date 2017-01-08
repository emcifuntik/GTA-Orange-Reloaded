#pragma once
namespace rage {
	class CVehicleScanner
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

		DWORD dwFlags; //0x0008 
		char pad_0x000C[0x4]; //0x000C
		rage::CVehicle* nearestArray[16]; //0x0010 
		DWORD dwArrayCount; //0x0090 
		char pad_0x0094[0x8]; //0x0094
		DWORD dwArraySize; //0x009C 
		DWORD dwLifeTime; //0x00A0 
		char pad_0x00A4[0x4]; //0x00A4

	}; //Size=0x00A8
}