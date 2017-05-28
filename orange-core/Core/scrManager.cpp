#include "stdafx.h"
#include "Windowsx.h"

#pragma comment(lib, "winmm.lib")

ScriptManagerThread g_ScriptManagerThread;

static HANDLE		mainFiber;
static Script *		currentScript;
scriptMap			ScriptManagerThread::m_scripts;

bool m_mouseButtonStates[3];

void Script::Tick() 
{
	if (mainFiber == nullptr)
		mainFiber = ConvertThreadToFiber(nullptr);

	if (timeGetTime() < wakeAt)
		return;

	if (scriptFiber) 
	{
		currentScript = this;
		SwitchToFiber(scriptFiber);
		currentScript = nullptr;
	}
	else
	{
		scriptFiber = CreateFiber(NULL, [](LPVOID handler) {
			//__try {
				reinterpret_cast<Script*>(handler)->Run();
			/*}
			__except (EXCEPTION_EXECUTE_HANDLER) {
				printStack(std::cout);
				log_error << "Error in script->Run. Callstack was written to " << std::endl;
			}*/
		}, this);
	}
}

void Script::Run()
{
	callbackFunction();
}

void Script::Yield(uint32_t time)
{
	wakeAt = timeGetTime() + time;
	SwitchToFiber(mainFiber);
}

void ScriptManagerThread::DoRun()
{
	for (auto & pair : m_scripts)
		pair.second->Tick();
}

eThreadState ScriptManagerThread::Reset(uint32_t scriptHash, void * pArgs, uint32_t argCount)
{
	scriptMap tempScripts;
	for (auto && pair : m_scripts)
		tempScripts[pair.first] = pair.second;
	m_scripts.clear();
	for (auto && pair : tempScripts)
		AddScript(pair.first, pair.second->GetCallbackFunction());
	return ScriptThread::Reset(scriptHash, pArgs, argCount);
}

void ScriptManagerThread::AddScript(std::string threadName, void(*fn)())
{
	//log_debug << "Registering thread " << threadName.c_str() << " 0x" << std::hex << fn << std::endl;
	if (m_scripts.find(threadName) != m_scripts.end()) 
	{
		log_error << "Thread " << threadName.c_str() << " is already registered" << std::endl;
		return;
	}
	m_scripts[threadName] = std::make_shared<Script>(fn);
}

void ScriptManagerThread::RemoveScript(std::string threadName) {

	auto pair = m_scripts.find(threadName);
	if (pair == m_scripts.end()) {
		log_error << "Could not find thread " << threadName << std::endl;
		return;
	}
	//log_debug << "Unregistered script " << threadName << std::endl;
	m_scripts.erase(pair);
}

void scriptWait(unsigned long waitTime) 
{
	currentScript->Yield(waitTime);
}

void scriptRegister(std::string threadName, void(*function)()) 
{
	g_ScriptManagerThread.AddScript(threadName, function);
}

void scriptUnregister(std::string threadName)
{
	g_ScriptManagerThread.RemoveScript(threadName);
}

int32_t getGameVersion()
{
	log_info << "getGameVersion not realized yet" << std::endl;
	return 0;
}

static ScriptManagerContext g_context;
static uint64_t g_hash;

void nativePush64(UINT64 value)
{
	g_context.Push(value);
}

void nativeInit(UINT64 hash)
{
	g_context.Reset();
	g_hash = hash;
}

uint64_t * nativeCall()
{
	auto fn = ScriptEngine::GetNativeHandler(g_hash);
	if (fn != 0) {
		__try {
			//log << "_Called 0x" << g_hash << std::endl;
			fn(&g_context);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			log_error << "Error in nativeCall. 0x" << g_hash << std::endl;
		}
	}
	return reinterpret_cast<uint64_t*>(g_context.GetResultPointer());
}


static std::set<TKeyboardFn> g_keyboardFunctions;

void keyboardHandlerRegister(TKeyboardFn function)
{
	g_keyboardFunctions.insert(function);
}

void keyboardHandlerUnregister(TKeyboardFn function)
{
	g_keyboardFunctions.erase(function);
}

bool isKeyDown(WPARAM wParam)
{
	return (GetKeyState(wParam) & 0x8000) != 0;
}

int GetCefKeyboardModifiers(WPARAM wParam, LPARAM lParam)
{
	int modifiers = 0;
	if (isKeyDown(VK_SHIFT))
		modifiers |= EVENTFLAG_SHIFT_DOWN;
	if (isKeyDown(VK_CONTROL))
		modifiers |= EVENTFLAG_CONTROL_DOWN;
	if (isKeyDown(VK_MENU))
		modifiers |= EVENTFLAG_ALT_DOWN;

	// Low bit set from GetKeyState indicates "toggled".
	if (::GetKeyState(VK_NUMLOCK) & 1)
		modifiers |= EVENTFLAG_NUM_LOCK_ON;
	if (::GetKeyState(VK_CAPITAL) & 1)
		modifiers |= EVENTFLAG_CAPS_LOCK_ON;

	switch (wParam) {
	case VK_RETURN:
		if ((lParam >> 16) & KF_EXTENDED)
			modifiers |= EVENTFLAG_IS_KEY_PAD;
		break;
	case VK_INSERT:
	case VK_DELETE:
	case VK_HOME:
	case VK_END:
	case VK_PRIOR:
	case VK_NEXT:
	case VK_UP:
	case VK_DOWN:
	case VK_LEFT:
	case VK_RIGHT:
		if (!((lParam >> 16) & KF_EXTENDED))
			modifiers |= EVENTFLAG_IS_KEY_PAD;
		break;
	case VK_NUMLOCK:
	case VK_NUMPAD0:
	case VK_NUMPAD1:
	case VK_NUMPAD2:
	case VK_NUMPAD3:
	case VK_NUMPAD4:
	case VK_NUMPAD5:
	case VK_NUMPAD6:
	case VK_NUMPAD7:
	case VK_NUMPAD8:
	case VK_NUMPAD9:
	case VK_DIVIDE:
	case VK_MULTIPLY:
	case VK_SUBTRACT:
	case VK_ADD:
	case VK_DECIMAL:
	case VK_CLEAR:
		modifiers |= EVENTFLAG_IS_KEY_PAD;
		break;
	case VK_SHIFT:
		if (isKeyDown(VK_LSHIFT))
			modifiers |= EVENTFLAG_IS_LEFT;
		else if (isKeyDown(VK_RSHIFT))
			modifiers |= EVENTFLAG_IS_RIGHT;
		break;
	case VK_CONTROL:
		if (isKeyDown(VK_LCONTROL))
			modifiers |= EVENTFLAG_IS_LEFT;
		else if (isKeyDown(VK_RCONTROL))
			modifiers |= EVENTFLAG_IS_RIGHT;
		break;
	case VK_MENU:
		if (isKeyDown(VK_LMENU))
			modifiers |= EVENTFLAG_IS_LEFT;
		else if (isKeyDown(VK_RMENU))
			modifiers |= EVENTFLAG_IS_RIGHT;
		break;
	case VK_LWIN:
		modifiers |= EVENTFLAG_IS_LEFT;
		break;
	case VK_RWIN:
		modifiers |= EVENTFLAG_IS_RIGHT;
		break;
	}
	return modifiers;
}

void ScriptManager::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_KEYDOWN || uMsg == WM_KEYUP || uMsg == WM_CHAR || uMsg == WM_SYSCHAR || uMsg == WM_SYSKEYDOWN || uMsg == WM_SYSKEYUP)
	{
		CefKeyEvent keyEvent;
		keyEvent.windows_key_code = wParam;
		keyEvent.native_key_code = lParam;
		keyEvent.modifiers = GetCefKeyboardModifiers(wParam, lParam);
		keyEvent.is_system_key = uMsg == WM_SYSCHAR || uMsg == WM_SYSKEYDOWN || uMsg == WM_SYSKEYUP;

		if (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN)
			keyEvent.type = cef_key_event_type_t::KEYEVENT_RAWKEYDOWN;
		else if (uMsg == WM_KEYUP || uMsg == WM_SYSKEYUP)
			keyEvent.type = cef_key_event_type_t::KEYEVENT_KEYUP;
		else if (uMsg == WM_CHAR || uMsg == WM_SYSCHAR)
			keyEvent.type = cef_key_event_type_t::KEYEVENT_CHAR;

		for (auto host : CEFCore::Get()->views)
			host->m_pWebView->GetHost()->SendKeyEvent(keyEvent);

	}
	else if (CNetworkUI::Get()->CursorShown() && (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP || uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP || uMsg == WM_MBUTTONUP || uMsg == WM_MBUTTONDOWN))
	{
		CefMouseEvent mouseEvent;

		mouseEvent.x = LOWORD(lParam);
		mouseEvent.y = HIWORD(lParam);

		cef_mouse_button_type_t btn;
		bool state;

		if (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP)
			btn = MBT_LEFT;
		else if (uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP)
			btn = MBT_RIGHT;
		else
			btn = MBT_MIDDLE;

		state = uMsg == WM_LBUTTONUP || uMsg == WM_RBUTTONUP || uMsg == WM_MBUTTONUP;

		m_mouseButtonStates[btn] = state;

		for (auto host : CEFCore::Get()->views)
			host->m_pWebView->GetHost()->SendMouseClickEvent(mouseEvent, btn, state, 1);
		
	}
	else if (CNetworkUI::Get()->CursorShown() && uMsg == WM_MOUSEMOVE)
	{
		CefMouseEvent mouseEvent;

		mouseEvent.x = LOWORD(lParam);
		mouseEvent.y = HIWORD(lParam);

		if (m_mouseButtonStates[MBT_LEFT])
			mouseEvent.modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
		if (m_mouseButtonStates[MBT_MIDDLE])
			mouseEvent.modifiers |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
		if (m_mouseButtonStates[MBT_RIGHT])
			mouseEvent.modifiers |= EVENTFLAG_RIGHT_MOUSE_BUTTON;

		for (auto host : CEFCore::Get()->views)
			host->m_pWebView->GetHost()->SendMouseMoveEvent(mouseEvent, false);
		
	}
	else if (CNetworkUI::Get()->CursorShown() && uMsg == WM_MOUSEWHEEL)
	{
		int delta = GET_WHEEL_DELTA_WPARAM(wParam);

		CefMouseEvent mouseEvent;

		mouseEvent.x = GET_X_LPARAM(lParam);
		mouseEvent.y = GET_Y_LPARAM(lParam);

		for (auto host : CEFCore::Get()->views)
			host->m_pWebView->GetHost()->SendMouseWheelEvent(mouseEvent, 0, delta);

	}
	else if (uMsg == WM_CLOSE)
		TerminateProcess(GetCurrentProcess(), 0);

	if (uMsg == WM_KEYDOWN || uMsg == WM_KEYUP || uMsg == WM_SYSKEYDOWN || uMsg == WM_SYSKEYUP)
	{
		auto functions = g_keyboardFunctions;
		for (auto & function : functions)
			function((DWORD)wParam, lParam & 0xFFFF, (lParam >> 16) & 0xFF, (lParam >> 24) & 1, (uMsg == WM_SYSKEYDOWN || uMsg == WM_SYSKEYUP), (lParam >> 30) & 1, (uMsg == WM_SYSKEYUP || uMsg == WM_KEYUP));
	}
}