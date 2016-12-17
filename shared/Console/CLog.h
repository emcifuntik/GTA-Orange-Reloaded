#pragma once
#ifndef _IOLOG_
#define _IOLOG_

#define _O_LOG_DEBUG "[Debug] "
#define _O_LOG_ERROR "[Error] "
#define _O_LOG_INFO "[Info] "

#if !defined PROJECT_NAME
#define PROJECT_NAME ""
#endif

class my_ostream
{
public:
	static std::string fname;
	static void SetLogFile(std::string fileName)
	{
		fname = fileName;
	}

	my_ostream() {
		my_fstream = std::ofstream(fname, std::ios_base::app);
		my_fstream << std::endl;
	};

	my_ostream& info()
	{
		std::cout << color::lblue << _O_LOG_INFO << color::white;
		my_fstream << _O_LOG_INFO;
		return *this;
	}
	my_ostream& debug()
	{
		std::cout << color::lyellow << _O_LOG_DEBUG << color::white;
		my_fstream << _O_LOG_DEBUG;
		return *this;
	}
	my_ostream& error()
	{
		std::cout << color::lred << _O_LOG_ERROR << color::white;
		my_fstream << _O_LOG_ERROR;
		return *this;
	}
	template<typename T> my_ostream& operator<<(const T& something)
	{
		std::cout << something;
		my_fstream << something;
		return *this;
	}
	typedef std::ostream& (*stream_function)(std::ostream&);
	my_ostream& operator<<(stream_function func)
	{
		func(std::cout);
		func(my_fstream);
		return *this;
	}
	static my_ostream& _log()
	{
		static my_ostream log_stream;
		time_t rawtime;
		tm timeinfo;
		char buffer[80];
		time(&rawtime);
		errno_t error = localtime_s(&timeinfo, &rawtime);
		if (error)
			return log_stream;
		strftime(buffer, 80, "[%Ex %EX]", &timeinfo);
		log_stream << color::lwhite << buffer << " " << color::white;
		return log_stream;
	}
private:
	std::ofstream my_fstream;
};

static std::string DateTimeA()
{
	time_t rawtime;
	tm timeinfo;
	char buffer[80];
	time(&rawtime);
	errno_t error = localtime_s(&timeinfo, &rawtime);
	if (error)
		return std::string("");
	strftime(buffer, 80, "[%Ex %EX]", &timeinfo);
	return buffer;
}

static std::wstring DateTimeW()
{
	time_t rawtime;
	tm timeinfo;
	char buffer[80];
	time(&rawtime);
	errno_t error = localtime_s(&timeinfo, &rawtime);
	if (error)
		return std::wstring(L"");
	strftime(buffer, 80, "[%Ex %EX]", &timeinfo);
	std::string buff(buffer);
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring wide = converter.from_bytes(buff);
	return wide;
}

#define log my_ostream::_log()
#define log_info my_ostream::_log().info()
#define log_debug my_ostream::_log().debug()
#define log_error my_ostream::_log().error()

#endif
