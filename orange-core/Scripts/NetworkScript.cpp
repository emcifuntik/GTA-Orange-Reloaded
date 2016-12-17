#include "stdafx.h"

ULONGLONG lastSendTick = 0;

void NetworkAction()
{
	for (;;)
	{
		CLocalPlayer::Get()->Tick();
		if (CNetworkConnection::Get()->IsConnected()) {
			if (CNetworkConnection::Get()->IsConnectionEstablished())
			{
				CLocalPlayer::Get()->SendOnFootData();
				if (CLocalPlayer::Get()->updateTasks)
				{
					CLocalPlayer::Get()->updateTasks ^= 1;
					//CLocalPlayer::Get()->SendTasks();
				}
			}
		}
		if (GetTickCount64() >= (lastSendTick+10))
		{
			if (CNetworkConnection::Get()->IsConnected()) {
				CNetworkConnection::Get()->Tick();
				CNetworkPlayer::Tick();
			}
			lastSendTick = GetTickCount64();
		}
		CNetworkUI::Get()->Render();
		scriptWait(0);
	}
}

SCRIPT("networkScript", NetworkAction, networkAction);