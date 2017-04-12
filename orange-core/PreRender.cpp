#include "stdafx.h"

void PreRenderAction()
{
	for (;;)
	{
		CNetworkPlayer::PreRender();
		CNetwork3DText::PreRender();
		scriptWait(0);
	}
}

SCRIPT(PreRenderAction)