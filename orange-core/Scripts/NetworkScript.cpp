#include "stdafx.h"

ULONGLONG lastSendTick = 0;

void NetworkAction()
{
	keyboardHandlerRegister(CNetworkUI::ScriptKeyboardMessage);
	for (;;)
	{
		CLocalPlayer::Get()->Tick();
		if (CNetworkConnection::Get()->IsConnected()) {
			if (CNetworkConnection::Get()->IsConnectionEstablished())
			{
				CLocalPlayer::Get()->SendOnFootData();
				/*if (CLocalPlayer::Get()->updateTasks)
				{
					CLocalPlayer::Get()->updateTasks = false;
					CLocalPlayer::Get()->SendTasks();
				}*/
			}
		}
		if (GetTickCount64() >= (lastSendTick + 10))
		{
			if (CNetworkConnection::Get()->IsConnected())
				CNetworkConnection::Get()->Tick();
			lastSendTick = GetTickCount64();
		}
		CNetworkPlayer::Tick();
		CNetworkVehicle::Tick();
		CNetworkPlayer::PreRender();
		CNetwork3DText::PreRender();
		CNetworkUI::Get()->Render();
		scriptWait(0);
	}
}

SCRIPT(NetworkAction);