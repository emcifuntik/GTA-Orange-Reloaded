// stdafx.h: включаемый файл дл€ стандартных системных включаемых файлов
// или включаемых файлов дл€ конкретного проекта, которые часто используютс€, но
// не часто измен€ютс€
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN

#include "lua.hpp"

#pragma region core
#include <cstdint>
#include <windows.h>
#include <string>
#include <sstream>
#include <Psapi.h>
#include <sstream>
#include <intrin.h>
#include <vector>
#include <io.h>
#include <thread>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <fstream>
#include <mutex>
#include <iostream>
#include <locale>
#include <codecvt>
#include <TimeAPI.h>
#include <shlobj.h>
#include <functional>
#include <queue>
#include "CVector3.h"
#include "Utils.h"
#include "Console\CConsole.h"
#include "Console\CLogClient.h"
#include <cfloat>
#include <cstdarg> 
#include <cstddef>
#include <cstring>
#include <Winternl.h>
#include <intrin.h>
#include <TlHelp32.h>
#include "tinyxml2.h"
#include "resource.h"
#include "font_awesome.h"
#pragma endregion

#include "Registry.h"

#include "D3D11/Memory/minhook/src/HDE/table64.h"
#include "D3D11/Memory/minhook/src/HDE/hde64.h"
typedef hde64s HDE;
#define HDE_DISASM(code, hs) hde64_disasm(code, hs)

#include "D3D11/Memory/minhook/src/HDE/pstdint.h"
#include "D3D11/Memory/minhook/src/buffer.h"
#include "D3D11/Memory/minhook/src/trampoline.h"
#include "D3D11/Memory/minhook/include/MinHook.h"
#include "D3D11/Memory/GameMemory.h"
#include "D3D11/Memory/Memory.h"


#pragma region D3D Stuff
#include "D3D11\Include\d3d11.h"
#include "D3D11\Include\d3dcompiler.h"
#include "D3D11\Include\dxgi1_2.h"
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma endregion

#pragma region CustomHook
#include "core/types.h"
#include "core/pgCollection.h"
#include "core/pgPtrCollection.h"
#include "core/nativeInvoker.h"
#include "core/scrThread.h"
#include "core/scrManager.h"
#include "core/scrEngine.h"
#include "core/nativeCaller.h"
#include "core/Script.h"
#include "core/Natives.h"
#pragma endregion

#pragma region RakNet
#include <MessageIdentifiers.h>
#include <RakPeerInterface.h>
#include <RakNetStatistics.h>
#include <RakNetTypes.h>
#include <BitStream.h>
#include <RakSleep.h>
#include <PacketLogger.h>
#include <Kbhit.h>
#include <Gets.h>
#include <WindowsIncludes.h>
#include <GetTime.h>
#include <RPC4Plugin.h>
#include <PacketizedTCP.h>
using namespace RakNet;
#pragma endregion

#pragma region Shared
#include "CMath.h"
#include "NetworkTypes.h"
#include "Models.h"
#pragma endregion

#pragma region RAGE Stuff
#include "GTA\sysAllocator.h"
#include "GTA\VTasks.h"
#include "GTA\CRage.h"
#include "GTA\CReplayInterface.h"
#include "GTA\CGraphics.h"
#pragma endregion

#pragma region Network
#include "Network\CConfig.h"
#include "Network\CEntity.h"
#include "Network\CPedestrian.h"
#include "Network\CVehicle.h"
#include "Network\CNetworkBlip.h"
#include "Network\CNetworkPlayer.h"
#include "Network\CNetworkVehicle.h"
#include "Network\CNetworkObject.h"
#include "Network\CNetworkConnection.h"
#include "Network\CNetworkUI.h"
#include "Network\CNetworkMarker.h"
#include "Network\CNetwork3DText.h"
#include "Network\CLocalPlayer.h"
#include "Network\Functions.h"
#include "Network\CRPCPlugin.h"
#pragma endregion

#include "Game.h"


#pragma region Scripting
#include "Scripting/CScriptEngine.h"
#pragma endregion

#pragma region IMGUI
#include "D3D11/Memory/Memory.h"
#include "thirdparty/DirectX/Include/DXGIFormat.h"
#include "thirdparty/DirectX/Include/DXGI.h"
#include "thirdparty/dxgitype.h"
#include "thirdparty/dxgi1_2.h"
#include "thirdparty/DirectX/Include/d3d11.h"
#include "D3D11/d3dhook.h"
#include "D3D11/imgui.h"
#include "D3D11/imgui_impl_dx11.h"
#include "GuiDispatcher.h"
#pragma endregion

#include "Core/Chat.h"
#include "Core/Commands.h"

#include "Core/ScriptInvoker.h"
#include "Core/Natives.h"
#include "MemoryCache.h"
#include "Memory.h"
#include "Globals.h"
#include "GtaScripts.h"
#include "orange-core.h"
#include "Utils.h"

IMGUI_API LRESULT ImGui_ImplDX11_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void CreateRenderTarget();
std::string getHWID();
std::string getHWIDClear();

enum eGameState {
	GameStatePlaying,
	GameStateIntro,
	GameStateLicenseShit = 3,
	GameStateMainMenu = 5,
	GameStateLoadingSP_MP = 6
};

typedef bool(*LookAlive)();
typedef bool(*GameStateChange_)();
static LookAlive g_origLookAlive;
static GameStateChange_ g_gameStateChange;

struct Color
{
	int r;
	int g;
	int b;
	int a;

	Color(int ar, int ag, int ab, int aa)
	{
		r = ar;
		g = ag;
		b = ab;
		a = aa;
	}
	UINT32 ToUINT32()
	{
		return ((UINT32)((((a) & 0xff) << 24) | (((b) & 0xff) << 16) | (((g) & 0xff) << 8) | ((r) & 0xff)));
	}
};

static LPARAM Icon;

#pragma region script macro
#define SCRIPT(z) class __script__##z:\
public CScript\
{\
public:\
	__script__##z() : CScript(""__FILE__"") {}\
protected:\
	virtual void Run() override { scriptRegister(""__FILE__"", z); }\
} __script__##z;
#pragma endregion
// USAGE: SCRIPT(function);

#pragma region gui macro
#define GUI(z) class __gui__##z\
{\
public:\
	__gui__##z(){CGuiDispatcher::Get() += z;}\
} __gui__##z;
#pragma endregion
// USAGE: GUI(function);

#define TRACE() log_debug << __FILE__ << " -> Line " << std::dec << __LINE__ << std::endl
