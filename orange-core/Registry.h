#pragma once
namespace Registry {
	int CreateRegKeyStructure(HKEY hKey, LPCTSTR sPath);
	bool CreateRegistryKey(HKEY hKeyRoot, LPCTSTR pszSubKey);
	bool DeleteRegistryKey(HKEY hKeyRoot, LPCTSTR pszSubKey);
	bool DeleteRegistryValue(HKEY hKeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue);
	bool Get_BinaryRegistryValue(HKEY hKeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue, PVOID pBuffer, DWORD& rdwSize);
	bool Get_BOOLRegistryValue(HKEY hKeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue);
	bool Get_DWORDRegistryValue(HKEY hKeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue, unsigned long &ulBuff);
	bool Get_DWORDRegistryValue(HKEY hKeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue, unsigned __int64 &u64Buff);
	bool Get_StringRegistryValue(HKEY hKeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue, LPTSTR pszBuffer, DWORD& rdwSize);
	bool Get_StringRegistryValue(HKEY hKeyRoot, const char* pszSubKey, const char* pszValue, const char* pszBuffer, DWORD& rdwSize);
	bool Set_BinaryRegistryValue(HKEY hKeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue, PVOID pData, DWORD dwSize);
	bool Set_BOOLRegistryValue(HKEY hKeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue, bool bVal);
	bool Set_DWORDRegistryValue(HKEY hKeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue, unsigned long ulValue);
	bool Set_DWORDRegistryValue(HKEY hKeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue, unsigned __int64 u64Buff);
	bool Set_StringRegistryValue(HKEY hKeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue, LPCTSTR pszString);
}