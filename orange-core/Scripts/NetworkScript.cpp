#include "stdafx.h"

ULONGLONG lastSendTick = 0;

void NetworkAction()
{
	keyboardHandlerRegister(CNetworkUI::ScriptKeyboardMessage);
	CScriptEngine::Get()->Init();
	for (;;)
	{
		CLocalPlayer::Get()->Tick();
		if (CNetworkConnection::Get()->IsConnected()) {
			if (CNetworkConnection::Get()->IsConnectionEstablished())
			{
				CLocalPlayer::Get()->SendOnFootData();
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
				CNetworkConnection::Get()->Tick();
			}
			lastSendTick = GetTickCount64();
		}
		CNetworkPlayer::Tick();
		CNetworkVehicle::Tick();
		CNetworkObject::Tick();

		CNetworkPlayer::PreRender();
		CNetwork3DText::PreRender();

		CNetworkUI::Get()->Render();
		CScriptEngine::Get()->Tick();

		scriptWait(0);
	}
}

SCRIPT(NetworkAction);