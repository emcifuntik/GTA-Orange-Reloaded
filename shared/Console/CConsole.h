#pragma once

namespace color {
	inline std::ostream& blue(std::ostream &s)
	{
#ifdef _WIN32
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hStdout, FOREGROUND_BLUE);
#else
		s << "\033[34m";
#endif
		return s;
	}

	inline std::ostream& lblue(std::ostream &s)
	{
#ifdef _WIN32
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hStdout, FOREGROUND_BLUE
			| FOREGROUND_INTENSITY);
#else
		s << "\033[94m";
#endif
		return s;
	}

	inline std::ostream& red(std::ostream &s)
	{
#ifdef _WIN32
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hStdout,
			FOREGROUND_RED);
#else
		s << "\033[31m";
#endif
		return s;
	}

	inline std::ostream& lred(std::ostream &s)
	{
#ifdef _WIN32
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hStdout,
			FOREGROUND_RED | FOREGROUND_INTENSITY);
#else
		s << "\033[91m";
#endif
		return s;
	}

	inline std::ostream& green(std::ostream &s)
	{
#ifdef _WIN32
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hStdout,
			FOREGROUND_GREEN);
#else
		s << "\033[32m";
#endif
		return s;
	}

	inline std::ostream& lgreen(std::ostream &s)
	{
#ifdef _WIN32
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hStdout,
			FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#else
		s << "\033[92m";
#endif
		return s;
	}

	inline std::ostream& yellow(std::ostream &s)
	{
#ifdef _WIN32
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hStdout,
			FOREGROUND_GREEN | FOREGROUND_RED);
#else
		s << "\033[33m";
#endif
		return s;
	}

	inline std::ostream& lyellow(std::ostream &s)
	{
#ifdef _WIN32
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hStdout,
			FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
#else
		s << "\033[93m";
#endif
		return s;
	}

	inline std::ostream& white(std::ostream &s)
	{
#ifdef _WIN32
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hStdout,
			FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else
		s << "\033[37m";
#endif
		return s;
	}

	inline std::ostream& lwhite(std::ostream &s)
	{
#ifdef _WIN32
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hStdout,
			FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
#else
		s << "\033[97m";
#endif
		return s;
	}

	struct color {
		color(WORD attribute) :m_color(attribute) {};
		WORD m_color;
	};

	template <class _Elem, class _Traits>
	std::basic_ostream<_Elem, _Traits>&
		operator<<(std::basic_ostream<_Elem, _Traits>& i, color& c)
	{
#ifdef _WIN32
		HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hStdout, c.m_color);
#endif
		return i;
	}
}
