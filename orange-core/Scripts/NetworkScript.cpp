#include "stdafx.h"

ULONGLONG lastSendTick = 0;

void NetworkAction()
{
	keyboardHandlerRegister(CNetworkUI::ScriptKeyboardMessage);
	for (;;)
	{
		CLocalPlayer::Get()->Tick(); TRACE();
		if (CNetworkConnection::Get()->IsConnected()) {
			if (CNetworkConnection::Get()->IsConnectionEstablished())
			{
				CLocalPlayer::Get()->SendOnFootData(); TRACE();
				/*if (CLocalPlayer::Get()->updateTasks)
				{
				CLocalPlayer::Get()->updateTasks ^= 1;
				CLocalPlayer::Get()->SendTasks();
				}*/
			}
		}
		if (GetTickCount64() >= (lastSendTick + 10))
		{
			if (CNetworkConnection::Get()->IsConnected()) {
				CNetworkConnection::Get()->Tick(); TRACE();
				CNetworkPlayer::Tick(); TRACE();
			}
			lastSendTick = GetTickCount64(); TRACE();
		}
		CNetworkVehicle::Tick(); TRACE();
		CNetworkPlayer::PreRender(); TRACE();
		CNetwork3DText::PreRender(); TRACE();
		CNetworkUI::Get()->Render(); TRACE();
		scriptWait(0);
	}
}

SCRIPT("networkScript", NetworkAction, networkAction);