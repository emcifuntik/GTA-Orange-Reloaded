#pragma once
namespace rage {
	class CPedIntelligenceFactory
	{
	public:
		CVector3 N00000625; //0x0000 
		char pad_0x000C[0x4]; //0x000C
		CVector3 N00000627; //0x0010 
		char pad_0x001C[0x14]; //0x001C
		CVector3 vecIntelligentPos; //0x0030 
		char pad_0x003C[0x44]; //0x003C
		rage::CDefensiveArea defensiveArea; //0x0080 
		rage::CEventScanner eventScanner; //0x0110 
		char pad_0x0300[0x3C]; //0x0300
		float fUnknown10; //0x033C 
		char pad_0x0340[0x18]; //0x0340
		rage::CPedTaskManager pedTaskManager; //0x0358 
		rage::CEventGroupPed eventGroupPed; //0x0398 
		rage::CPed* owner; //0x0438 
		char pad_0x0440[0x110]; //0x0440
		rage::CVehicleScanner vehicleScanner; //0x0550 
		rage::CPedScanner pedScanner; //0x05F8 
		rage::CObjectScanner objectScanner; //0x0698 
		char pad_0x0738[0x10]; //0x0738
		DWORD dwUnknown80; //0x0748 
		char pad_0x074C[0x214]; //0x074C
		rage::CDoorScanner doorScanner; //0x0960 
		char pad_0x0A00[0x298]; //0x0A00
		rage::CRelationshipGroup* relationshipGroup; //0x0C98 
		char pad_0x0CA0[0x10]; //0x0CA0
		float floatArray1[16]; //0x0CB0 
		char pad_0x0D30[0x350]; //0x0D30

	}; //Size=0x0048
}