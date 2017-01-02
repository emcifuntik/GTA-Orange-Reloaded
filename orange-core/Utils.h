#pragma once
namespace Utils {
	unsigned int Hash(const char* inpStr);
	std::string GetCurDir();
	std::string UnicodeToMultibyte(const std::wstring& wstrs);
	std::wstring MultibyteToUnicode(const std::string& str);
	int RoundToBytes(int bits);
	DWORD RGBAToHex(BYTE r, BYTE g, BYTE b, BYTE a);
	void HexToRGBA(DWORD hexValue, BYTE &r, BYTE &g, BYTE &b, BYTE &a);
	bool ReadRegistry(HKEY hKeyLocation, const char * szLocation, const char * szRow, const char *szBuffer, DWORD dwSize);
	bool WriteRegistry(HKEY hKeyLocation, const char * szSubKey, const char * szKey, const char * szData, DWORD dwSize);
	CVector3 RotationToDirection(CVector3 rotation);
};
