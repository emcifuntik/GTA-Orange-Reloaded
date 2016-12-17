#include "stdafx.h"

int CFolderBrowser::BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED)
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	return 0;
}

CFolderBrowser::CFolderBrowser(std::wstring title)
{
	TCHAR dlgTitle[128];
	wcscpy_s(dlgTitle, 128, title.c_str());
	bi.lpszTitle = dlgTitle;
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = (LPARAM)"";
}


CFolderBrowser::~CFolderBrowser()
{
}

bool CFolderBrowser::Show()
{
	pidl = SHBrowseForFolder(&bi);
	return pidl != 0;
}

std::wstring CFolderBrowser::GetPath()
{
	if (pidl == 0)
		throw new std::exception("Show function not called");
	TCHAR path[MAX_PATH];
	SHGetPathFromIDList(pidl, path);

	IMalloc * imalloc = 0;
	if (SUCCEEDED(SHGetMalloc(&imalloc)))
	{
		imalloc->Free(pidl);
		imalloc->Release();
	}
	return std::wstring(path);
}
