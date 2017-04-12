#include "stdafx.h"

void ScriptingAction()
{
	for (;;)
	{
		CScriptEngine::Get()->Tick();
		scriptWait(0);
	}
}

SCRIPT(ScriptingAction);