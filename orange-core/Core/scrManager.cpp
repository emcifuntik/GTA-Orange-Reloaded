#include "stdafx.h"

#include "DbgHelp.h"
#include <WinBase.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Dbghelp.lib")

void printStack(std::fstream& out)
{
	typedef USHORT(WINAPI *CaptureStackBackTraceType)(__in ULONG, __in ULONG, __out PVOID*, __out_opt PULONG); TRACE();
	CaptureStackBackTraceType func = (CaptureStackBackTraceType)(GetProcAddress(LoadLibrary(L"kernel32.dll"), "RtlCaptureStackBackTrace")); TRACE();

	if (func == NULL)
		return; // WOE 29.SEP.2010
	TRACE();
				// Quote from Microsoft Documentation:
				// ## Windows Server 2003 and Windows XP:  
				// ## The sum of the FramesToSkip and FramesToCapture parameters must be less than 63.
	const int kMaxCallers = 62; TRACE();

	void         * callers_stack[kMaxCallers];
	unsigned short frames;
	SYMBOL_INFO  * symbol;
	HANDLE         process;
	process = GetCurrentProcess(); TRACE();
	SymInitialize(process, NULL, TRUE); TRACE();
	frames = (func)(0, kMaxCallers, callers_stack, NULL); TRACE();
	symbol = (SYMBOL_INFO *)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1); TRACE();
	symbol->MaxNameLen = 255; TRACE();
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO); TRACE();

	const unsigned short  MAX_CALLERS_SHOWN = 6; TRACE();
	frames = frames < MAX_CALLERS_SHOWN ? frames : MAX_CALLERS_SHOWN; TRACE();
	for (unsigned int i = 0; i < frames; i++)
	{
		SymFromAddr(process, (DWORD64)(callers_stack[i]), 0, symbol); TRACE();
		out << "*** " << i << ": " << callers_stack[i] << " " << symbol->Name << " - 0x" << symbol->Address << std::endl; TRACE();
	}
	TRACE();
	free(symbol); TRACE();
}

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
			try {
				reinterpret_cast<Script*>(handler)->Run();
			}
			catch (...) {
				std::string fname = DateTimeA() + ".call_stack.txt";
				std::ofstream callStackFile(fname);
				printStack(*(std::fstream*)&callStackFile);
				log_error << "Error in script->Run. Callstack was written to " << fname << std::endl;
			}
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