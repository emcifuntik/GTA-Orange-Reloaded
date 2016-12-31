#include "stdafx.h"

void ScriptDispatcherAction()
{
	for(;;)
	{
		while (!CScriptInvoker::Get().Empty())
			CScriptInvoker::Get().Pop()();
		scriptWait(0);
	}
}

SCRIPT(ScriptDispatcherAction);
