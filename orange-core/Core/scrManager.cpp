#include "stdafx.h"

#pragma comment(lib, "winmm.lib")

ScriptManagerThread g_ScriptManagerThread;

static HANDLE		mainFiber;
static Script *		currentScript;
scriptMap			ScriptManagerThread::m_scripts;

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
	log_debug << "Registering thread " << threadName.c_str() << " 0x" << std::hex << fn << std::endl;
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
	log_debug << "Unregistered script " << threadName << std::endl;
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
			fn(&g_context);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			log_error << "Error in nativeCall" << std::endl;
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

void ScriptManager::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_KEYDOWN || uMsg == WM_KEYUP || uMsg == WM_SYSKEYDOWN || uMsg == WM_SYSKEYUP)
	{
		auto functions = g_keyboardFunctions;
		for (auto & function : functions)
			function((DWORD)wParam, lParam & 0xFFFF, (lParam >> 16) & 0xFF, (lParam >> 24) & 1, (uMsg == WM_SYSKEYDOWN || uMsg == WM_SYSKEYUP), (lParam >> 30) & 1, (uMsg == WM_SYSKEYUP || uMsg == WM_KEYUP));
	}
}