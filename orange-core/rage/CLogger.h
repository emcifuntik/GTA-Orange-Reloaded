#pragma once
namespace rage {
	class CLogger: 
		public CLoggerBase {
	public:
		//virtual ~CLogger() {}
		virtual void Unused0() { log_debug << __FUNCTION__ << std::endl; }
		virtual void Unused1() { log_debug << __FUNCTION__ << std::endl; }
		virtual void Unused2() { log_debug << __FUNCTION__ << std::endl; }
		virtual void Unused3() { log_debug << __FUNCTION__ << std::endl; }
		virtual void Unused4() { log_debug << __FUNCTION__ << std::endl; }
		virtual void Unused5() { log_debug << __FUNCTION__ << std::endl; }
		virtual void Write(int unused, const char * format, ...) { 
			char dest[128];
			va_list argptr;
			va_start(argptr, format);
			vsprintf_s(dest, 128, format, argptr);
			va_end(argptr);
			log << "rage::CLogger: " << dest << std::endl;
		}
		virtual void Unused6() { log_debug << __FUNCTION__ << std::endl; }
		virtual void Unused7() { log_debug << __FUNCTION__ << std::endl; }
		virtual void Unused8() { log_debug << __FUNCTION__ << std::endl; }
		virtual void Unused9() { log_debug << __FUNCTION__ << std::endl; }
		virtual void Unused10() { log_debug << __FUNCTION__ << std::endl; }

	};
}