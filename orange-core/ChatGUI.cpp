#include "stdafx.h"

void ChatGui()
{
	if (CGlobals::Get().showChat)
		CChat::Get()->Render();
}

//GUI(ChatGui);