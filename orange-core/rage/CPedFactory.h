#pragma once
namespace rage {
	class CPedFactory
	{
	public:
		virtual ~CPedFactory() = 0;
		virtual rage::CPed * Create(unsigned char *a2, int *a3, int64_t a4, bool someFlag, bool network, bool controllable) = 0; //
		virtual int64_t Function1(int64_t a2, int64_t a3, int64_t a4, int a5, bool a6) = 0; //
		virtual int64_t Function2(int64_t a2, int64_t a3, bool a4, bool a5) = 0; //
		virtual void Function3(int64_t a2, int64_t a3, bool a4) = 0; //
		virtual int64_t Function4(unsigned char *a2, int64_t a3, int64_t a4, int64_t a5) = 0; //
		virtual int Function5(int64_t a2) = 0; //
		rage::CPed* localPed; //0x0008 
	}; //Size=0x0010
};