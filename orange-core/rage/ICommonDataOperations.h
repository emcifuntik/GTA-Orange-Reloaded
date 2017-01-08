#pragma once
namespace rage {
	class ICommonDataOperations
	{
	public:
		virtual int Read(void *target, rage::CBuffer buffer); //
		virtual int Write(void *target, rage::CBuffer buffer, void* unknown1, bool unknown2); //
	private:
		virtual void Unknown(); //
	public:
		virtual int64_t GetSize(); //
		virtual int Debug(void* out); //
		virtual int Debug(void * unknown1, void * unknown2); //

		rage::netSyncDataNode* target; //0x0008 
		char pad_0x0010[0x8]; //0x0010

	}; //Size=0x0018
}