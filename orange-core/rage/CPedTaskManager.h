#pragma once
namespace rage {
	class CPedTaskManager
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

		rage::CTaskTree* Primary; //0x0008 
		rage::CTaskTree* Secondary; //0x0010 
		rage::CTaskTree* Movement; //0x0018 
		rage::CTaskTree* Motion; //0x0020 
		DWORD TaskTrees; //0x0028 
		char pad_0x002C[0x4]; //0x002C
		unsigned char bUnknown1; //0x0030 
		char pad_0x0031[0x7]; //0x0031
		rage::CPed* owner; //0x0038 

	}; //Size=0x0040
}