#include "stdafx.h"

#define CHECK_PARAMS(n) { if (params[0] != (n * sizeof(cell))) { API::Get().Print("SCRIPT: Bad parameter count"); return 0; } }
/*
*
* ALL NATIVE FUNCTIONS
*
*/

// native print(const string[])
static cell AMX_NATIVE_CALL n_print(AMX* amx, cell* params)
{
	CHECK_PARAMS(1);

	char* msg;
	amx_StrParam(amx, params[1], msg);
	return 0;
}

static cell AMX_NATIVE_CALL n_setPlayerPosition(AMX* amx, cell* params)
{
	CHECK_PARAMS(3);

	API::Get().SetPlayerPosition(params[0], params[1], params[2], params[3]);
	return 0;
}
AMX_NATIVE_INFO custom_Natives[] =
{
	// Util
	{ "print",					n_print },

	// Player
	{ "setPlayerPosition",		n_setPlayerPosition },

	{ NULL, NULL }
};

int CFunctions::amx_CustomInit(AMX *amx)
{
	return amx_Register(amx, custom_Natives, -1);
}

CFunctions *CFunctions::Get()
{
	if (!singleInstance) {
		singleInstance = new CFunctions();
	}
	return singleInstance;
}