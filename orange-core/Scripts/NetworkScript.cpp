#include "stdafx.h"

ULONGLONG lastSendTick = 0;
ULONGLONG lastSendTime = 0;

void NetworkAction()
{
	keyboardHandlerRegister(CNetworkUI::ScriptKeyboardMessage);
	keyboardHandlerRegister(CChat::ScriptKeyboardMessage);
	for (;;)
	{
		CLocalPlayer::Get()->Tick();
		if (CNetworkConnection::Get()->IsConnected()) {
			if (CNetworkConnection::Get()->IsConnectionEstablished() &&
				(timeGetTime() > (lastSendTime + 50) || CLocalPlayer::Get()->IsShooting()))
			{
				CLocalPlayer::Get()->SendOnFootData();
				lastSendTime = timeGetTime();

				/*if (CLocalPlayer::Get()->updateTasks)
				{
					CLocalPlayer::Get()->updateTasks ^= 1;
					CLocalPlayer::Get()->SendTasks();
				}*/
			}
		}

		if (CNetworkConnection::Get()->IsConnected()) {
			CNetworkConnection::Get()->Tick();
		}

		CNetworkPlayer::Tick();
		CNetworkVehicle::Tick();
		CNetworkObject::Tick();
		CNetworkBlip::Tick();

		CNetworkUI::Get()->Render();

		scriptWait(0);
	}
}

SCRIPT(NetworkAction);