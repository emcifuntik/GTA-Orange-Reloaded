#include "stdafx.h"

namespace PlayerNatives
{
	SQInteger GetPlayerName(HSQUIRRELVM v)
	{
		SQInteger playerId;
		sq_getinteger(v, -1, &playerId);
		CNetworkPlayer * player = CNetworkPlayer::GetByID(playerId);
		if (!player)
		{
			sq_pushstring(v, "", 0);
			return 1;
		}
		else
		{
			sq_pushstring(v, player->GetName().c_str(), player->GetName().length());
			return 1;
		}
	}

	SQInteger SetPlayerPos(HSQUIRRELVM v)
	{
		SQInteger playerId;
		sq_getinteger(v, -4, &playerId);
		SQFloat x, y, z;
		sq_getfloat(v, -3, &x);
		sq_getfloat(v, -2, &y);
		sq_getfloat(v, -1, &z);
		auto player = CNetworkPlayer::GetByID(playerId);
		if (!player)
			return SQFalse;
		else
		{
			player->SetPosition(CVector3(x, y, z));
			return SQTrue;
		}
	}
}

void RegisterScriptFunctions()
{
	Squirrel::AddFunction((SQFUNCTION)PlayerNatives::GetPlayerName, "getPlayerName");
	Squirrel::AddFunction((SQFUNCTION)PlayerNatives::SetPlayerPos, "setPlayerPos");
}
