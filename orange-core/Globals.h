#pragma once

typedef void(*ForceCleanupForAllThreadsWithThisName_)(const char * name, int mask);
typedef void(*TerminateAllScriptsWithThisName_)(const char * name);
typedef void(*ShutdownLoadingScreen_)();
typedef void(*InitializeOnline_)();
typedef void(*DoScreenFadeIn_)(int64_t delay);
typedef bool(*HasScriptLoaded_)(const char * name);
typedef bool(*SetMenuState_)(const char * name);

struct FreemodeCustomize {
	int Blemishes = 0;
	bool BlemishesOff;
	int Facial_Hair = 0;
	bool Facial_HairOff;
	int Eyebrows = 0;
	bool EyebrowsOff;
	int Ageing = 0;
	bool AgeingOff;
	int Makeup = 0;
	bool MakeupOff;
	int Blush = 0;
	bool BlushOff;
	int Complexion = 0;
	bool ComplexionOff;
	int Sun_Damage = 0;
	bool Sun_DamageOff;
	int Lipstick = 0;
	bool LipstickOff;
	int Moles_Freckles = 0;
	bool Moles_FrecklesOff;
	int Chest_Hair = 0;
	bool Chest_HairOff;
	int Body_Blemishes = 0;
	bool Body_BlemishesOff;
	int Add_Body_Blemishes = 0;
	bool Add_Body_BlemishesOff;
};

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
	FreemodeCustomize lastCustom;
	FreemodeCustomize custom;
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
	ID3D11Device *d3dDevice = nullptr;
	ID3D11DeviceContext *d3dDeviceContext = nullptr;
	IDXGISwapChain *d3dSwapChain = nullptr;
	ID3D11RenderTargetView* d3dRenderTargetView = nullptr;
	ImFont *chatFont = nullptr;
	bool renderer = false;
	bool *hudDisabled = nullptr;
	bool displayServerBrowser = false;
	bool displayCustomizeWindow = false;
	bool mainmenushown = false;
	bool showChat = false;
	bool blockquickswitch = true;
	int currentGameState = -1;
	char serverIP[32] = "";
	char nickName[32] = "";
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

