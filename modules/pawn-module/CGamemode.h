#pragma once

class CGameMode
{
private:
	AMX m_amx;
	bool m_bInitialised;
	bool m_bSleeping;
	float m_fSleepTime;

public:
	CGameMode();
	~CGameMode();

	static CGameMode *singleInstance;
	static CGameMode *Get();

	bool Load(char* pFileName);

	int OnPlayerConnect(cell playerid);
	int OnPlayerDisconnect(cell playerid);
	int OnPlayerUpdate(cell playerid);
};