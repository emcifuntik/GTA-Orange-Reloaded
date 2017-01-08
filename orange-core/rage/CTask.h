#pragma once
namespace rage {
	class CTask
	{
	public:
		virtual ~CTask();
		virtual int64_t GetID();
		virtual rage::CTask* Clone();
		virtual bool SetSubTask_(int someInt, rage::CTask* childTask);
		virtual bool IsSimple();
		virtual void unknown_5();
		virtual void unknown_6();
		virtual void unknown_7();
		virtual void unknown_8();
		virtual void unknown_9();
		virtual void unknown_10();
		virtual void unknown_11();
		virtual void unknown_12();
		virtual void unknown_13();
		virtual void unknown_14();
		virtual void unknown_15();
		virtual void unknown_16();
		virtual void unknown_17();
		virtual void unknown_18();
		virtual void unknown_19();
		virtual void unknown_20();
		virtual void unknown_21();
		virtual void unknown_22();
		virtual void unknown_23();
		virtual void unknown_24();
		virtual void unknown_25();
		virtual void unknown_26();
		virtual void unknown_27();
		virtual void unknown_28();
		virtual void unknown_29();
		virtual void unknown_30();
		virtual void unknown_31();
		virtual void unknown_32();
		virtual void unknown_33();
		virtual void unknown_34();
		virtual void unknown_35();
		virtual void unknown_36();
		virtual void unknown_37();
		virtual void unknown_38();
		virtual void unknown_39();
		virtual void unknown_40();
		virtual void unknown_41();
		virtual void unknown_42();
		virtual void unknown_43();
		virtual void unknown_44();
		virtual bool IsSerializable();
		virtual CSerialisedFSMTaskInfo* Serialize();
		virtual void unknown_47();
		virtual void unknown_48();
		virtual void unknown_49();
		virtual void unknown_50();
		virtual bool returnZero64_1();
		virtual bool returnZero64_2();
		virtual bool returnTrue_1();
		virtual bool returnZero64_3();
		virtual bool returnMinusOne_1();
		virtual bool returnFalse_1();
		virtual void unknown_57();
		virtual void unknown_58();
		virtual void unknown_59();
		virtual void unknown_60();
		virtual void unknown_61();
		virtual bool returnFalse_2();
		virtual bool returnFalse_3();
		virtual bool returnFalse_4();
		virtual bool returnFalse_5();
		virtual bool returnFalse_6();
		virtual bool returnTrue_2();
		virtual bool returnFalse_7();
		virtual bool returnTrue_3();
		virtual int Deserialize(void* clonedTask);

		char pad_0x0008[0x8]; //0x0008
		rage::CEntity* Owner; //0x0010 
		rage::CTask* Parent; //0x0018 
		rage::CTask* Child; //0x0020 
		rage::CTask* NextChild; //0x0028 
		int32_t iUnknown1; //0x0030 
		int32_t iUnknown2; //0x0034 
		float timeFromBegin; //0x0038 
		float totalTime; //0x003C 
		char pad_0x0040[0x50]; //0x0040
		std::string GetTree(rage::CTask *task = nullptr, int n = 0)
		{
			if (!n)
			{
				task = this;
				return VTasks::Get()->GetTaskName(this->GetID()) + GetTree(task->Child, n + 1);
			}
			else
			{
				std::string res("\n");
				if (!task)
					return res;
				for (int i = 0; i < n; ++i)
					res += "-";
				res += " ";
				res += VTasks::Get()->GetTaskName(task->GetID());
				return res + GetTree(task->Child, n + 1);
			}
		}
	}; //Size=0x0090

}