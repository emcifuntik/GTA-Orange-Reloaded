#include "stdafx.h"

void ScriptDispatcherAction()
{
	for(;;)
	{
		D3DHook::SetRender(!(UI::IS_PAUSE_MENU_ACTIVE() || UI::_0xE18B138FABC53103()));
		while (!CScriptInvoker::Get().Empty())
			CScriptInvoker::Get().Pop()();
		scriptWait(0);
	}
}

SCRIPT(ScriptDispatcherAction);
