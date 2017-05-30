#include "stdafx.h"

extern "C" int amx_CoreInit(AMX* amx);
extern "C" int amx_CoreCleanup(AMX* amx);
extern "C" int amx_FloatInit(AMX* amx);
extern "C" int amx_FloatCleanup(AMX* amx);
extern "C" int amx_StringInit(AMX* amx);
extern "C" int amx_StringCleanup(AMX* amx);
extern "C" int amx_FileInit(AMX* amx);
extern "C" int amx_FileCleanup(AMX* amx);
extern "C" int amx_TimeInit(AMX* amx);
extern "C" int amx_TimeCleanup(AMX* amx);
extern "C" int amx_DGramInit(AMX* amx);
extern "C" int amx_DGramCleanup(AMX* amx);

char szGameModeFileName[256];

CGameMode::CGameMode()
{
	m_bInitialised = false;
	m_bSleeping = false;
}

CGameMode::~CGameMode()
{
	// should be unloaded
}

bool CGameMode::Load(char* pFileName)
{

	FILE* f = fopen(pFileName, "rb");
	if (!f) return false;
	fclose(f);

	memset((void*)&m_amx, 0, sizeof(AMX));
	m_fSleepTime = 0.0f;
	strcpy(szGameModeFileName, pFileName);

	int err = CScripts::Get()->aux_LoadProgram(&m_amx, szGameModeFileName);
	if (err != AMX_ERR_NONE)
	{
		CScripts::Get()->AMXPrintError(this, &m_amx, err);
		API::Get().Print("Failed to load script.");
		return false;
	}

	amx_CoreInit(&m_amx);
	amx_FloatInit(&m_amx);
	amx_StringInit(&m_amx);
	amx_FileInit(&m_amx);
	amx_TimeInit(&m_amx);
	CFunctions::Get()->amx_CustomInit(&m_amx);


	m_bInitialised = true;

	// Execute OnGameModeInit callback, if it exists!
	int tmp;
	if (!amx_FindPublic(&m_amx, "OnGameModeInit", &tmp))
		amx_Exec(&m_amx, (cell*)&tmp, tmp);
	// ----------------------------------------------

	cell ret = 0;
	err = amx_Exec(&m_amx, &ret, AMX_EXEC_MAIN);
	if (err == AMX_ERR_SLEEP)
	{
		m_bSleeping = true;
		m_fSleepTime = ((float)ret / 1000.0f);
	}
	else if (err != AMX_ERR_NONE)
	{
		m_bSleeping = false;
		CScripts::Get()->AMXPrintError(this, &m_amx, err);
	}

	return true;
}

// forward OnPlayerConnect(playerid);
int CGameMode::OnPlayerConnect(cell playerid)
{
	int idx;
	cell ret = 0;

	if (!amx_FindPublic(&m_amx, "OnPlayerConnect", &idx))
	{
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

// forward OnPlayerDisconnect(playerid);
int CGameMode::OnPlayerDisconnect(cell playerid)
{

	int idx;
	cell ret = 0;

	if (!amx_FindPublic(&m_amx, "OnPlayerDisconnect", &idx))
	{
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

// forward OnPlayerUpdate(playerid);
int CGameMode::OnPlayerUpdate(cell playerid)
{
	int idx;
	cell ret = 1;

	if (!amx_FindPublic(&m_amx, "OnPlayerUpdate", &idx))
	{
		amx_Push(&m_amx, playerid);
		amx_Exec(&m_amx, &ret, idx);
	}
	return (int)ret;
}

CGameMode *CGameMode::Get()
{
	if (!singleInstance) {
		singleInstance = new CGameMode();
	}
	return singleInstance;
}