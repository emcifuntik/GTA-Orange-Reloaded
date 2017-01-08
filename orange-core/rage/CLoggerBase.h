#pragma once
namespace rage {
	class CLoggerBase {
	public:
		//virtual ~CLoggerBase() = 0;
		virtual void Unused0() = 0;
		virtual void Unused1() = 0;
		virtual void Unused2() = 0;
		virtual void Unused3() = 0;
		virtual void Unused4() = 0;
		virtual void Unused5() = 0;
		virtual void Write(int unused, const char * format, ...) = 0;
		virtual void Unused6() = 0;
		virtual void Unused7() = 0;
		virtual void Unused8() = 0;
		virtual void Unused9() = 0;
		virtual void Unused10() = 0;
	};
}