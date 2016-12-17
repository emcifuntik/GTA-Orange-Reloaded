#pragma once
class CFolderBrowser
{
	BROWSEINFO bi = { 0 };
	static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
	LPITEMIDLIST pidl;
public:
	CFolderBrowser(std::wstring title);
	~CFolderBrowser();

	bool Show();
	std::wstring GetPath();
};

