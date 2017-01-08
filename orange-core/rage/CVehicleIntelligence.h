#pragma once
namespace rage {
	class CVehicleIntelligence
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

		rage::CVehicleScanner vehicleScanner; //0x0008 
		rage::CPedScanner pedScanner; //0x00B0 
		rage::CObjectScanner objectScanner; //0x0150 
		char pad_0x01F0[0x5C0]; //0x01F0
		rage::CVehicleTaskManager taskManager; //0x07B0 
		char pad_0x0920[0x1E8]; //0x0920

	}; //Size=0x0AC8
}