#include "stdafx.h"

typedef void(*InitialMount_)();
InitialMount_ g_callInitialMount;

CMemory unusedMem;

bool ScriptsDisabled = false;

void ForceToSingle()
{
	CMemory mem((uintptr_t)GetModuleHandle(NULL) + 0x2773C); //48 83 EC 28 85 D2 78 71 75 0F
	CMemory mem2((uintptr_t)GetModuleHandle(NULL) + 0x186680); //48 83 EC 28 B9 ? ? ? ? E8 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? B1 01
	(mem + 0x3B).put(DWORD(mem2() - mem() - 0x3F));
}

void UnknownPatches()
{
	CMemory mem((uintptr_t)GetModuleHandle(NULL) + 0x1B348B); //48 85 C9 0F 84 ? 00 00 00 48 8D 55 A7 E8
	auto mem2 = mem + 13;
	mem2.put(0x01B0i16);
	mem2.nop(3);
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x1AE3A0).nop(5); //E8 ? ? ? ? 8B CB 40 88 2D ? ? ? ?
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x1E6EF8).retn(); //48 89 5C 24 ? 57 48 83 EC 20 8B F9 8B DA
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x11CD8C4).retn();
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x11D03D0).retn();
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x1C220E).nop(9);
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0xF8A528).retn();
	(CMemory((uintptr_t)GetModuleHandle(NULL) + 0x11D986C) - 4).retn();

	mem = CMemory((uintptr_t)GetModuleHandle(NULL) + 0x23AD9C); //HECK_MULTIPLAYER_BYTE_DRAW_MAP_FRAME
	mem2 = CMemory(mem);
	mem.nop(7);
	mem2.put(0xB640i16);
	mem2.put(0x01i8);

	(CMemory((uintptr_t)GetModuleHandle(NULL) + 0x141A3) + 2).put(0x08i8);
	(CMemory((uintptr_t)GetModuleHandle(NULL) + 0xA64CA6) - 74).retn();
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x1AE3A0).nop(5);
}

void DefineNatives()
{
	CGlobals::Get().ForceCleanupForAllThreadsWithThisName = 
		(ForceCleanupForAllThreadsWithThisName_)((uintptr_t)GetModuleHandle(NULL) + 0xC70970); //FORCE_CLEANUP_FOR_ALL_THREADS_WITH_THIS_NAME
	CGlobals::Get().TerminateAllScriptsWithThisName = 
		(TerminateAllScriptsWithThisName_)((uintptr_t)GetModuleHandle(NULL) + 0xA3DAE8); //TerminateAllScriptsWithThisName
	CGlobals::Get().ShutdownLoadingScreen = 
		(ShutdownLoadingScreen_)((uintptr_t)GetModuleHandle(NULL) + 0x1FBD34); //ShutdownLoadingScreen
	CGlobals::Get().DoScreenFadeIn = 
		(DoScreenFadeIn_)((uintptr_t)GetModuleHandle(NULL) + 0x2A1554); //DoScreenFadeIn
	CGlobals::Get().HasScriptLoaded = 
		(HasScriptLoaded_)((uintptr_t)GetModuleHandle(NULL) + 0xCE37E0); //HasScriptLoaded
	CGlobals::Get().canLangChange = (bool*)((uintptr_t)CMemory((uintptr_t)GetModuleHandle(NULL) + 0x1C183F).getOffset(2) + 1);
	CGlobals::Get().InitializeOnline = (InitializeOnline_)((uintptr_t)GetModuleHandle(NULL) + 0x103708);
}

static bool OnLookAlive()
{
	static bool HUDInited = false;
	if (!HUDInited)
	{
		typedef void(*InitHUD)(void);
		InitHUD(CMemory((uintptr_t)GetModuleHandle(NULL) + 0x1F358F)() - 0x23)();
		HUDInited = true;
	}
	if (!ScriptsDisabled && CGlobals::Get().HasScriptLoaded("startup"))
	{
		//DisableScripts();
		CGlobals::Get().ForceCleanupForAllThreadsWithThisName("startup", 8);
		CGlobals::Get().TerminateAllScriptsWithThisName("startup");
		CGlobals::Get().ShutdownLoadingScreen();
		CGlobals::Get().DoScreenFadeIn(0);
	}
	//OnGameFrame
	return g_origLookAlive();
}


void TurnOnConsole()
{
	AllocConsole();
	SetConsoleTitle(L"Grand Theft Auto: Orange");
	FILE * unused = NULL;
	freopen_s(&unused, "CONOUT$", "w", stdout);
	freopen_s(&unused, "CONOUT$", "w", stderr);
}

LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ScriptManager::WndProc(hwnd, uMsg, wParam, lParam);
	ImGui_ImplDX11_WndProcHandler(hwnd, uMsg, wParam, lParam);
	return CallWindowProc(CGlobals::Get().gtaWndProc, hwnd, uMsg, wParam, lParam);
}

void __fastcall eventHook(rage::CTask* task)
{
	/*log_debug << task->GetTree() << std::endl;
	CLocalPlayer::Get()->updateTasks = true;*/
}

bool consoleShowed = false;
void OnGameStateChange(int gameState)
{
	switch (gameState)
	{
	case GameStateIntro:
		break;
	case GameStateLicenseShit:
		break;
	case GameStatePlaying:
	{
		TurnOnConsole();
		//SetWindowText(CGlobals::Get().gtaHwnd, L"GTA:Orange");
		if (!ScriptEngine::Initialize())
			log_error << "Failed to initialize ScriptEngine" << std::endl;
		D3DHook::HookD3D11();
		CChat::Get()->RegisterCommandProcessor(CommandProcessor);

		log_info << "Game ready" << std::endl;
		CGlobals::Get().gtaWndProc = (WNDPROC)SetWindowLongPtr(CGlobals::Get().gtaHwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
		if (CGlobals::Get().gtaWndProc == NULL)
			log_error << "Failed to attach input hook" << std::endl;
		else
			log_info << "Input hook attached: WndProc 0x" << std::hex << (DWORD_PTR)CGlobals::Get().gtaWndProc << std::endl;
		ScriptEngine::CreateThread(&g_ScriptManagerThread);
		CScript::RunAll();


		//typedef void(*InitNetStuff_t)();
		//InitNetStuff_t((uintptr_t)GetModuleHandle(NULL) + 0x1F891E0)();// CMemory::Find("48 89 5C 24 08 57 48 83 EC 40 33 FF 40 38 3D ? ? ? ?")();
		SyncTree::Init();

		(CMemory((uintptr_t)GetModuleHandle(NULL) + 0x12F8862) + 35).nop(9);
		CMemory((uintptr_t)GetModuleHandle(NULL) + 0x7FFF0C).farJmp(eventHook);
		break;
	}
	case GameStateMainMenu:
		break;
	}
}

static bool gameStateChange_(int gameState)
{
	OnGameStateChange(gameState);
	CGlobals::Get().currentGameState = gameState;
	return g_gameStateChange();
}


static HWND CreateWindowExWHook(_In_ DWORD dwExStyle,
	_In_opt_ LPCWSTR lpClassName,
	_In_opt_ LPCWSTR lpWindowName,
	_In_ DWORD dwStyle,
	_In_ int X,
	_In_ int Y,
	_In_ int nWidth,
	_In_ int nHeight,
	_In_opt_ HWND hWndParent,
	_In_opt_ HMENU hMenu,
	_In_opt_ HINSTANCE hInstance,
	_In_opt_ LPVOID lpParam)
{
	Icon = (LPARAM)LoadIcon(CGlobals::Get().dllModule,MAKEINTRESOURCE(IDI_ICON1));
	HWND hWnd = CreateWindowExW(dwExStyle, lpClassName, L"GTA:Orange", dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	SendMessage(hWnd, WM_SETICON, ICON_BIG, Icon);
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, Icon);
	CGlobals::Get().gtaHwnd = hWnd;
	return hWnd;
}

static void callInitialMount_()
{
	g_callInitialMount();
	rage::fiDeviceRelative::MountFolder((CGlobals::Get().orangePath + "\\userdata\\").c_str(), "user:/", (unsigned char*)((uintptr_t)GetModuleHandle(NULL) + 0x1BED140));
	//rage::fiDeviceRelative::MountFolder((CGlobals::Get().orangePath + "\\orange\\").c_str(), "orange:/");

	//rage::fiDeviceRelative::MountFolder((CGlobals::Get().orangePath + "/config/").c_str(), "orange:/", (unsigned char*)((uintptr_t)GetModuleHandle(NULL) + 0x1BED140));
	/*rage::fiDeviceRelative* device = new rage::fiDeviceRelative();
	device->SetPath((CGlobals::Get().orangePath + "\\config\\").c_str(), false);
	device->Mount("orange:/");

	log_debug << "Mounted" << std::endl;*/
}

void HookLoop()
{
	unusedMem = CMemory((uintptr_t)GetModuleHandle(NULL) + 0x109D5D8);
	auto callToMem = unusedMem();
	unusedMem.farJmp(OnLookAlive);
	auto lookFrame = (CMemory((uintptr_t)GetModuleHandle(NULL) + 0x67A7) + 7);
	auto lookMem = lookFrame();
	g_origLookAlive = lookFrame.get_call<LookAlive>();
	(lookFrame + 1).put(DWORD(callToMem - lookMem - 5));

	callToMem = unusedMem();
	unusedMem.farJmp(gameStateChange_);
	auto gameStateChange = CMemory((uintptr_t)GetModuleHandle(NULL) + 0x1EC8FA); // GameStateChange
	auto gameStateMem = gameStateChange();
	g_gameStateChange = gameStateChange.get_call<GameStateChange_>();
	(gameStateChange + 1).put(DWORD(callToMem - gameStateMem - 5));

	callToMem = unusedMem();
	unusedMem.farJmp(CreateWindowExWHook);
	auto windowCreate = CMemory((uintptr_t)GetModuleHandle(NULL) + 0x12416F7); // WindowCreate
	auto windowCreateMem = windowCreate();
	windowCreate.nearCall(DWORD(callToMem - windowCreateMem - 5));
	windowCreate.nop(1);
	(gameStateChange + 1).put(long(callToMem - gameStateMem - 5));

	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x7B2A06).nop(37);

	callToMem = unusedMem();
	unusedMem.farJmp(callInitialMount_);
	auto callInitialMount = CMemory((uintptr_t)GetModuleHandle(NULL) + 0x7A521A); // CallInitialMount
	auto callInitialMountMem = callInitialMount();
	g_callInitialMount = callInitialMount.get_call<InitialMount_>();
	(callInitialMount + 1).put(long(callToMem - callInitialMountMem - 5));

	//CMemory((uintptr_t)GetModuleHandle(NULL) + 0x46A1BC).farJmp(gunShotHook);
}

void GameProcessHooks()
{
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0xC91D39).nop(24);//Objects
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x1F5BEC).nop(5); //Esc freeze
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x7B2F6C).retn(); //Cheat console
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x2413D2).nop(6); //UI Wheel slowmo
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x127C8CA).nop(4); //Show cursor
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x127C8DC).nop(4); //Show cursor
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x1BCCE8).retn(); //Rockstar loading logo
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x1C8E28).retn(); //Tooltips
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x10046F0).retn(); //Social club news
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x62039C).retn(); //Disable wanted generation
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x6194BE).put(0xE990i16); //Disable wanted generation 2
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x11D2BD3).nop(5); //Intentional crash
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0xCCC992).nop(23); //RASH_LOAD_MODELS_TOO_QUICKLY
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x109D5D4).retn(); //REATE_NETWORK_EVENT_BINDINGS
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x597484).retn(); //OAD_NEW_GAME
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0xEE29C8).retn(); //ESET_VEHICLE_DENSITY_LAST_FRAME
	(*(uint64_t*)CMemory((uintptr_t)GetModuleHandle(NULL) + 0xEE29D7).getOffset(2)) = 0; //AR_VEHICLE_DENSITY
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x5BB16C).retn(); //ET_CLOCK_FORWARD_AFTER_DEATH
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x1788CC).nop(46); //ISABLE_NORTH_BLIP
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0xEE22B8).retn(); //ISABLE_VEHICLE_RESET_AT_SET_POSITION
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x8F6624).retn(); //ISABLE_LOADING_MP_DLC_CONTENT
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x9F69D4).retn(); //UNTIME_EXECUTABLE_IMPORTS_CHECK
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0xE66118).retn(); //ISABLE_POPULATION_VEHICLES_10
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0xEF9670).retn(); //ISABLE_POPULATION_VEHICLES_8
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0xE9BE73).nop(3); //ISABLE_POPULATION_VEHICLES_11
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0xE3CC85).nop(5); //ISABLE_POPULATION_VEHICLES_11
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x442F4C).retn(); //ISABLE_POPULATION_PEDS_1
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x694030).retn(); //ISABLE_POPULATION_PEDS_2
	CMemory mem((uintptr_t)GetModuleHandle(NULL) + 0x6AC43D); //ISABLE_POPULATION_AMBIENT_PEDS
	(mem + 6).put(0x0i32);
	(mem + 16).put(0x0i32);
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x6BF525).nop(20); //ISABLE_POPULATION_PEDS_4
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x5FA314).retn(); //ISABLE_COPS_AND_FIRE_TRUCKS_1
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x33E78C).retn(); //ISABLE_COPS_AND_FIRE_TRUCKS_2
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x61F620).retn(); //ISABLE_COPS_AND_FIRE_TRUCKS_3
}

static HWND CreateWindowExWHook(_In_ DWORD dwExStyle,
	_In_opt_ LPCWSTR lpClassName,
	_In_opt_ LPCWSTR lpWindowName,
	_In_ DWORD dwStyle,
	_In_ int X,
	_In_ int Y,
	_In_ int nWidth,
	_In_ int nHeight,
	_In_opt_ HWND hWndParent,
	_In_opt_ HMENU hMenu,
	_In_opt_ HINSTANCE hInstance,
	_In_opt_ LPVOID lpParam)
{
	Icon = LPARAM(LoadIconA(NULL, (CGlobals::Get().orangePath + "/Launcher.ico").c_str()));
	HWND hWnd = CreateWindowExW(dwExStyle, lpClassName, L"GTA:Orange", dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	SendMessage(hWnd, WM_SETICON, ICON_BIG, Icon);
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, Icon);
	CGlobals::Get().gtaHwnd = hWnd;
	
	return hWnd;
}

void *OldCreateWindowExW = nullptr;

void EnableRageLogger()
{
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x156C7A4).nop(5);
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x156C874).nop(5);
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x156C844).nop(5);
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x156C814).nop(5);
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x156C7E4).nop(5);
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x156CB34).nop(5);
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x156CB04).nop(5);
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x156CAD4).nop(5);
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x156CAA4).nop(5);
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x156C991).nop(5);
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x156C8AE).nop(5);
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x156C9C8).nop(5);
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x156C90E).nop(5);
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x156CB69).nop(5);
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x156CA1C).nop(5);
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x156CA7E).nop(5);
}

void HookSetNetHandle()
{
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x65B0F0).nop(5);
	CMemory((uintptr_t)GetModuleHandle(NULL) + 0x65B12B).nop(24);
}

void PreLoadPatches()
{
	//strcpy_s((char*)((uintptr_t)GetModuleHandle(NULL) + 0x17F9C68), 32, "orange:/pausemenu.xml\0");


	ImGui::GetIO().IniFilename = (CGlobals::Get().orangePath + "\\imgui.ini").c_str();
	ImGui::GetIO().LogFilename = (CGlobals::Get().orangePath + "\\imgui_log.txt").c_str();

	auto mem = CMemory((uintptr_t)GetModuleHandle(NULL) + 0x14493);
	mem.put(0xEB90909090909090);

	EnableRageLogger();
	//HookSetNetHandle();
	DefineNatives();
	ForceToSingle();
	UnknownPatches();
	HookLoop();
	GameProcessHooks();
}
