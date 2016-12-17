#pragma once

class scriptHandlerMgr 
{
public:
	virtual ~scriptHandlerMgr();
	virtual void _Function1() = 0;
	virtual void _Function2() = 0;
	virtual void _Function3() = 0;
	virtual void _Function4() = 0;
	virtual void _Function5() = 0;
	virtual void _Function6() = 0;
	virtual void _Function7() = 0;
	virtual void _Function8() = 0;
	virtual void _Function9() = 0;
	virtual void AttachScript(scrThread * thread) = 0;
};

class ScriptEngine 
{
public:
	static bool Initialize();
	static pgPtrCollection<ScriptThread> * GetThreadCollection();
	static scriptHandlerMgr * GetScriptHandleMgr();
	static scrThread * GetActiveThread();
	static void SetActiveThread(scrThread * thread);
	static void CreateThread(ScriptThread * thread);
	typedef void(__cdecl * NativeHandler)(scrNativeCallContext * context);
	static NativeHandler GetNativeHandler(uint64_t oldHash);
};
