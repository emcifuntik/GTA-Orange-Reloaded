#include "stdafx.h"

eThreadState ScriptThread::Tick(uint32_t opsToExecute)
{
	typedef eThreadState(__thiscall * ScriptThreadTick_t)(ScriptThread * ScriptThread, uint32_t opsToExecute);
	ScriptThreadTick_t threadTickGta = (ScriptThreadTick_t)CMemory((uintptr_t)GetModuleHandle(NULL) + 0x9F646B).get<void>(-0xF);
	return threadTickGta(this, opsToExecute);
}

void ScriptThread::Kill()
{
	typedef void(__thiscall * ScriptThreadKill_t)(ScriptThread * ScriptThread);
	ScriptThreadKill_t killScriptThread = (ScriptThreadKill_t)CMemory((uintptr_t)GetModuleHandle(NULL) + 0x9ECF72).get<void>(-0x6);
	return killScriptThread(this);
}

eThreadState ScriptThread::Run(uint32_t opsToExecute)
{
	if (GetScriptHandler() == nullptr) 
	{
		ScriptEngine::GetScriptHandleMgr()->AttachScript(this);
		this->m_bNetworkFlag = true;
	}
	scrThread * activeThread = ScriptEngine::GetActiveThread();
	ScriptEngine::SetActiveThread(this);
	if (m_Context.m_State != ThreadStateKilled)
		DoRun();
	ScriptEngine::SetActiveThread(activeThread);
	return m_Context.m_State;
}

void ScriptThreadInit(ScriptThread * thread)
{
	typedef void(__thiscall * ScriptThreadInit_t)(ScriptThread * ScriptThread);
	auto scriptThreadInitPattern = CMemory((uintptr_t)GetModuleHandle(NULL) + 0x9EB4DC);//problem
	ScriptThreadInit_t ScriptThreadInit_ = (ScriptThreadInit_t)scriptThreadInitPattern();
	return ScriptThreadInit_(thread);
}

eThreadState ScriptThread::Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount)
{
	memset(&m_Context, 0, sizeof(m_Context));

	m_Context.m_State = ThreadStateIdle;
	m_Context.m_iScriptHash = scriptHash;
	m_Context.m_iUnk1 = -1;
	m_Context.m_iUnk2 = -1;
	m_Context.m_iSet1 = 1;

	ScriptThreadInit(this);

	m_pszExitMessage = "Normal exit";
	return m_Context.m_State;
}