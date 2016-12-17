#pragma once
namespace Utils {
	bool FileExist(std::wstring path);
	long FileSize(std::wstring filename);
	std::string GetCurDir();
	std::string UnicodeToMultibyte(const std::wstring& wstr, const char* localeName = "C");
	std::wstring MultibyteToUnicode(const std::string& str);
}

