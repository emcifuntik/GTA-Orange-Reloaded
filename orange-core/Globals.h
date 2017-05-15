#pragma once

typedef void(*ForceCleanupForAllThreadsWithThisName_)(const char * name, int mask);
typedef void(*TerminateAllScriptsWithThisName_)(const char * name);
typedef void(*ShutdownLoadingScreen_)();
typedef void(*InitializeOnline_)();
typedef void(*DoScreenFadeIn_)(int64_t delay);
typedef bool(*HasScriptLoaded_)(const char * name);
typedef bool(*SetMenuState_)(const char * name);
typedef int(*CreateObject_)(const char * name);

class CGlobals
{
	CGlobals() {}
	static CGlobals * singleInstance;
public:
	static CGlobals& Get()
	{
		if (!singleInstance)
			singleInstance = new CGlobals();
		return *singleInstance;
	}
	std::stringstream debugInfo;
	char chatBuffer[256];
	HMODULE dllModule = 0;
	LPVOID baseAddr = nullptr;
	std::string orangePath;
	bool alreadyRunned = false;
	bool *canLangChange = nullptr;
	HWND gtaHwnd = nullptr;
	WNDPROC gtaWndProc = nullptr;
	bool isSteam = false;
	bool isDeveloper = false;
	bool d3dloaded = false;
	Cam currentcam;
	CVector2 cefsize;
	ID3D11Device *d3dDevice = nullptr;
	ID3D11DeviceContext *d3dDeviceContext = nullptr;
	IDXGISwapChain *d3dSwapChain = nullptr;
	ID3D11RenderTargetView* d3dRenderTargetView = nullptr;
	ImFont *chatFont = nullptr;
	ImFont *tagFont = nullptr;
	ImFont *signpainterFont = nullptr;
	ImFont *londonFont = nullptr;
	ImFont *newyorkFont = nullptr;
	bool renderer = false;
	bool *hudDisabled = nullptr;
	bool displayServerBrowser = false;
	bool mainmenushown = false;
	bool showChat = false;
	bool blockquickswitch = true;
	int currentGameState = -1;
	char serverIP[32] = "109.167.153.69";
	//char serverIP[32] = "194.87.239.79";
	//char serverIP[32] = "123.56.158.105"; // China Server
	char nickName[32] = "";
	std::string name;
	int serverPort = 7788;
	bool isDebug = false;

	//VT Keymap
	const DWORD KEYOPENCHAT = VK_F7;
	const DWORD KEYMESSAGECHAT = VK_F6;

	ForceCleanupForAllThreadsWithThisName_ ForceCleanupForAllThreadsWithThisName;
	TerminateAllScriptsWithThisName_ TerminateAllScriptsWithThisName;
	ShutdownLoadingScreen_ ShutdownLoadingScreen;
	DoScreenFadeIn_ DoScreenFadeIn;
	HasScriptLoaded_ HasScriptLoaded;
	InitializeOnline_ InitializeOnline;
	SetMenuState_ SetMenuState;
};

