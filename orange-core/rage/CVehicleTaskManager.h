#pragma once
namespace rage {
	class CVehicleTaskManager
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

		rage::CTaskTree* primaryTasksPtr; //0x0008 
		rage::CTaskTree* secondaryTasksPtr; //0x0010 
		char pad_0x0018[0x20]; //0x0018
		rage::CTaskTree primaryTasks; //0x0038 
		rage::CTaskTree secondaryTasks; //0x0088 
		rage::CVehicle* owner; //0x00D8 
		char pad_0x00E0[0x90]; //0x00E0

	}; //Size=0x0130
}