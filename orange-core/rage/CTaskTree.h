#pragma once
namespace rage {
	class CTaskTree
	{
	public:
		virtual void Function0(); //
		virtual void Function1(); //
		virtual void AssignTask(rage::CTask *task, int taskPriority);
		virtual void Function3(); //
		virtual void Function4(); //
		virtual void Function5(); //
		virtual void Function6(); //
		virtual void Function7(); //
		virtual void Function8(); //
		virtual void Function9(); //

		rage::CEntity* owner; //0x0008 
		int32_t iActiveTask; //0x0010 
		int32_t iNextTask; //0x0014 
		int32_t iUnknown1; //0x0018 
		int32_t iUnknown2; //0x001C 
		rage::CTask* Tasks; //0x0020 

		rage::CTask* GetTask()
		{
			if (iActiveTask < 0)
				return nullptr;
			else
				return (&Tasks)[iActiveTask];
		}
	}; //Size=0x0030
}