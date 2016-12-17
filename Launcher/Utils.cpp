#include "stdafx.h"

namespace Utils {
	bool FileExist(std::wstring path)
	{
		return std::ifstream(path).good();
	}

	long FileSize(std::wstring filename)
	{
		return (long)std::wifstream(filename, std::wifstream::ate | std::wifstream::binary).tellg();
	}
	std::string GetCurDir()
	{
		CHAR CurPath[MAX_PATH];
		GetCurrentDirectoryA(MAX_PATH, CurPath);
		return std::string(CurPath);
	}
	std::wstring MultibyteToUnicode(const std::string& str)
	{
		DWORD minSize;
		minSize = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
		wchar_t unicodeStr[MAX_PATH];
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, unicodeStr, minSize);
		std::wstring wstr(unicodeStr);
		return wstr;
	}

	std::string UnicodeToMultibyte(const std::wstring& wstr, const char* localeName)
	{
		std::string result;
		result.resize(wstr.size());
		std::locale loc(localeName);
		std::use_facet<std::ctype<wchar_t> >(loc).narrow(
			wstr.c_str(), wstr.c_str() + wstr.size(), '?', &*result.begin());
		return result;
	}
}

