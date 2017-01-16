#include "stdafx.h"

namespace Utils {
	unsigned int Hash(const char* inpStr)
	{
		unsigned int value = 0, temp = 0;
		for (size_t i = 0; i<strlen(inpStr); i++)
		{
			temp = tolower(inpStr[i]) + value;
			value = temp << 10;
			temp += value;
			value = temp >> 6;
			value ^= temp;
		}
		temp = value << 3;
		temp += value;
		unsigned int temp2 = temp >> 11;
		temp = temp2 ^ temp;
		temp2 = temp << 15;
		value = temp2 + temp;
		if (value < 2) value += 2;
		return value;
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

	std::string UnicodeToMultibyte(const std::wstring& wstr)
	{
		char chr[MAX_PATH];
		size_t converted;
		wcstombs_s(&converted, chr, wstr.length(), wstr.c_str(), wstr.length());
		std::string s = std::string(chr);
		return s;
	}

	std::string utf8_encode(const std::wstring &wstr)
	{
		if (wstr.empty()) return std::string();
		int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
		std::string strTo(size_needed, 0);
		WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
		return strTo;
	}

	int RoundToBytes(int bits)
	{
		return (bits % 8) ? (bits / 8 + 1) : (bits / 8);
	}

	DWORD RGBAToHex(BYTE r, BYTE g, BYTE b, BYTE a)
	{
		return ((r & 0xff) << 24) + ((g & 0xff) << 16) + ((b & 0xff) << 8) + (a & 0xff);
	}

	void HexToRGBA(DWORD hexValue, BYTE & r, BYTE & g, BYTE & b, BYTE & a)
	{
		r = ((hexValue >> 24) & 0xFF);
		g = ((hexValue >> 16) & 0xFF);
		b = ((hexValue >> 8) & 0xFF);
		a = ((hexValue) & 0xFF);
	}

	void HexToRGB(DWORD hexValue, BYTE & r, BYTE & g, BYTE & b)
	{
		r = ((hexValue >> 16) & 0xFF);
		g = ((hexValue >> 8) & 0xFF);
		b = ((hexValue) & 0xFF);
	}

	bool ReadRegistry(HKEY hKeyLocation, const char * szLocation, const char * szRow, const char *szBuffer, DWORD dwSize)
	{
		HKEY hKey = NULL;
		if (RegOpenKeyExA(hKeyLocation, szLocation, NULL, KEY_READ, &hKey) == ERROR_SUCCESS)
		{
			DWORD dwType = REG_SZ;
			LONG getStatus = RegQueryValueExA(hKey, szRow, NULL, &dwType, (BYTE *)szBuffer, &dwSize);
			RegCloseKey(hKey);
			return (getStatus == ERROR_SUCCESS);
		}
		return false;
	}

	bool WriteRegistry(HKEY hKeyLocation, const char * szSubKey, const char * szKey, const char * szData, DWORD dwSize)
	{
		HKEY hKey = NULL;
		RegOpenKeyExA(hKeyLocation, szSubKey, NULL, KEY_ALL_ACCESS, &hKey);
		if (!hKey)
			RegCreateKeyExA(hKeyLocation, szSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
		if (hKey)
		{
			RegSetValueExA(hKey, szKey, NULL, REG_SZ, (BYTE *)szData, dwSize);
			RegCloseKey(hKey);
			return true;
		}
		return false;
	}

	double DegToRad(double deg)
	{
		return deg * PI / 180.0;
	}

	CVector3 RotationToDirection(CVector3 rotation)
	{
		auto z = DegToRad(rotation.fZ);
		auto x = DegToRad(rotation.fX);
		auto num = std::abs(std::cos(x));
		return CVector3((float)(-std::sin(z) * num), (float)(std::cos(z) * num), (float)std::sin(x));
	}
};