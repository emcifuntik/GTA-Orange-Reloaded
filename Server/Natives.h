#pragma once

namespace PlayerNatives
{
	SQInteger GetPlayerName(HSQUIRRELVM v);
	SQInteger SetPlayerPos(HSQUIRRELVM v);
}

void RegisterScriptFunctions();
