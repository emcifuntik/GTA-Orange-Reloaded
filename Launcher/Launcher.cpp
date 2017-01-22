// Launcher.cpp: определяет точку входа для приложения.
//

#include "stdafx.h"
#include "Launcher.h"
#include "Injector.h"
#include <thread>

#define MAX_LOADSTRING 100
#define PIRATE_EXECUTABLE_SIZE 54528512

// Глобальные переменные:
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING] = L"GTA:Orange Launcher";
WCHAR szWindowClass[MAX_LOADSTRING] = L"_gtaorange_launcher";
Image* pBitmap = NULL;
float loadProgress = 0.0;
HWND splashHwnd = NULL;
ULONG_PTR m_gdiplusToken;


ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void				UpdateSplash(float progress);

void UpdateSplash(float progress)
{
	if (progress == 1.0)
	{
		TerminateProcess(GetCurrentProcess(), 0);
		return;
	}
	loadProgress = progress;
	::RECT rect;
	rect.left = 0;
	rect.top = 200;
	rect.right = 300;
	rect.bottom = 226;
	InvalidateRect(splashHwnd, &rect, false);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LAUNCHER));

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
	Gdiplus::GdiplusShutdown(m_gdiplusToken);
    return (int) msg.wParam;
}


void LaunchGame()
{
	Registry::CreateRegKeyStructure(HKEY_CURRENT_USER, L"SOFTWARE\\GTA Orange Team\\GTA Orange");
	TCHAR curDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, curDir);
	Registry::Set_StringRegistryValue(HKEY_CURRENT_USER, L"SOFTWARE\\GTA Orange Team\\GTA Orange", L"OrangeFolder", curDir);

	std::string curPath = Utils::GetCurDir();
	Injector::Get().PushLibrary(curPath + "\\orange-core.dll");
	SetEnvironmentVariableA("PATH", curPath.c_str());

	bool isSteam = false;
	bool isPirate = false;

	UpdateSplash(0.33f);
	std::wstring gameFolder = L"";
	TCHAR TgameFolder[MAX_PATH];
	DWORD gameLen = MAX_PATH;

	if (!Registry::Get_StringRegistryValue(HKEY_CURRENT_USER, L"SOFTWARE\\GTA Orange Team\\GTA Orange", L"GameFolder", TgameFolder, gameLen))
	{
		CFolderBrowser folderBrowser(L"Select your GTA:V folder");
		bool folderSelected = folderBrowser.Show();
		if (!folderSelected)
		{
			TerminateProcess(GetCurrentProcess(), 0);
			return;
		}
		gameFolder = folderBrowser.GetPath();
		Registry::Set_StringRegistryValue(HKEY_CURRENT_USER, L"SOFTWARE\\GTA Orange Team\\GTA Orange", L"GameFolder", gameFolder.c_str());
	}
	else
		gameFolder = TgameFolder;

	std::wstring gamePath = gameFolder + L"\\GTA5.exe";

	int fs = (int)Utils::FileSize(gameFolder + L"\\GTA5.exe");
	if (fs == PIRATE_EXECUTABLE_SIZE)
		isPirate = true;
	if (Utils::FileExist(gameFolder + L"\\steam_api64.dll"))
		isSteam = true;

	if (!isSteam || isPirate)
		Injector::Get().Run(gameFolder, gamePath);
	else
		Injector::Get().RunSteam();
	UpdateSplash(0.70f);
	Injector::Get().InjectAll(!isPirate);
	UpdateSplash(1.0f);
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LAUNCHER));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_LAUNCHER));
	return RegisterClassEx(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
	pBitmap = Bitmap::FromResource(hInstance, MAKEINTRESOURCEW(IDB_BITMAP1));

	hInst = hInstance;

	RECT rect;
	GetClientRect(GetDesktopWindow(), &rect);
	rect.left = (rect.right / 2) - (300 / 2);
	rect.top = (rect.bottom / 2) - (300 / 2);
	splashHwnd = CreateWindowW(szWindowClass, L"", WS_OVERLAPPEDWINDOW,
		rect.left, rect.top, 300, 300, nullptr, nullptr, hInstance, nullptr);
	SetWindowLong(splashHwnd, GWL_STYLE, 0);

	if (!splashHwnd)
	{
		return FALSE;
	}

	ShowWindow(splashHwnd, nCmdShow);
	UpdateWindow(splashHwnd);
	std::thread thr(LaunchGame);
	thr.detach();

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		SwapBuffers(hdc);
		Graphics g(hdc);
		g.DrawImage(pBitmap, 0, 0);
		SolidBrush backPen(Gdiplus::Color(50, 231, 231, 231));
		SolidBrush frontPen(Gdiplus::Color(255, 130, 0));
		g.FillRectangle(&backPen, 67, 220, 168, 6);
		g.FillRectangle(&frontPen, 67, 220, (int)round(168 * loadProgress), 6);
		SwapBuffers(hdc);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		//PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
