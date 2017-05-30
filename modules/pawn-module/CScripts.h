#pragma once

#include "stdafx.h"

class CScripts
{
public:
	static CScripts *singleInstance;
	static CScripts *Get();

	char * aux_StrError(int errnum);
	void AMXPrintError(CGameMode* pGameMode, AMX *amx, int error);
	int aux_LoadProgram(AMX* amx, char* filename);
};