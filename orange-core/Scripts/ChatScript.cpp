#include "stdafx.h"

void ChatAction()
{
	keyboardHandlerRegister(CChat::ScriptKeyboardMessage);
	CChat::Get()->scaleform = GRAPHICS::REQUEST_SCALEFORM_MOVIE("multiplayer_chat");
	for (;;)
	{
		//CChat::Get()->Render();
		CChat::Get()->Input();
		scriptWait(0);
	}
}

SCRIPT(ChatAction);