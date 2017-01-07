#include "stdafx.h"

SResource *SResource::singleInstance = nullptr;

static const struct luaL_Reg gfunclib[] = {
	{ "print", lua_print },
	{ NULL, NULL }
};

static const struct luaL_Reg mfunclib[] = {
	{ "CreateBlipForAll", lua_CreateBlipForAll },
	{ "CreateBlipForPlayer", lua_CreateBlipForPlayer },
	{ "DeleteBlip", lua_DeleteBlip },
	{ "SetBlipColor", lua_SetBlipColor },
	{ "SetBlipRoute", lua_SetBlipRoute },

	{ "CreateMarkerForAll", lua_CreateMarkerForAll },
	{ "CreateMarkerForPlayer", lua_CreateMarkerForPlayer },
	{ "DeleteMarker", lua_DeleteMarker },

	{ "CreateVehicle", lua_CreateVehicle },

	{ "CreateObject", lua_CreateObject },

	{ "GetPlayerCoords", lua_GetPlayerCoords },
	{ "SetPlayerCoords", lua_SetPlayerCoords },
	{ "GetPlayerName", lua_GetPlayerName },
	{ "GetPlayerModel", lua_GetPlayerModel },
	{ "GivePlayerWeapon", lua_GivePlayerWeapon },
	{ "PlayerExists", lua_PlayerExists },
	{ "SetPlayerInfoMsg", lua_SetPlayerInfoMsg },
	{ "SetPlayerIntoVehicle", lua_SetPlayerIntoVehicle },
	
	{ "OnTick", lua_tick },
	{ "OnHTTPReq", lua_HTTPReq },
	{ "OnEvent", lua_Event },
	{ "OnCommand", lua_Command },
	{ "SQLEnv", luaopen_luasql_mysql },

	{ "Create3DText", lua_Create3DText },
	{ "Set3DTextText", lua_Set3DTextText },
	{ "Attach3DTextToVeh", lua_Attach3DTextToVeh },
	{ "Attach3DTextToPlayer", lua_Attach3DTextToPlayer },

	{ NULL, NULL }
};

SResource *SResource::Get()
{
	if (!singleInstance) {
		singleInstance = new SResource();
	}
	return singleInstance;
}
SResource::SResource()
{
}

bool SResource::Init()
{
	m_lua = luaL_newstate();
	luaJIT_setmode(m_lua, 0, LUAJIT_MODE_ENGINE | true);

	luaL_openlibs(m_lua);

	lua_getglobal(m_lua, "_G");
	luaL_setfuncs(m_lua, gfunclib, 0);
	lua_pop(m_lua, 1);

	lua_newtable(m_lua);
	luaL_setfuncs(m_lua, mfunclib, 0);
	lua_setglobal(m_lua, "__orange__");

	if (luaL_loadfile(m_lua, "modules//lua-module//API.lua") || lua_pcall(m_lua, 0, 0, 0)) {
		std::stringstream ss;
		ss << "[LUA] " << lua_tostring(m_lua, -1);
		API::Get().Print(ss.str().c_str());
		return false;
	}
	return true;
}

bool SResource::Start(const char* name)
{
	char path[64];
	char respath[64];

	sprintf(path, "resources//%s//", name);
	sprintf(respath, "%smain.lua", path);

	std::stringstream ss;
	ss << "[LUA] Starting resource " << name;
	API::Get().Print(ss.str().c_str());
	
	if (luaL_loadfile(m_lua, respath) || lua_pcall(m_lua, 0, 0, 0)) {
		std::stringstream ss;
		ss << "[LUA] Could not load main.lua for resource:\n\t" << lua_tostring(m_lua, -1) << name;
		API::Get().Print(ss.str().c_str());
		return false;
	}
	return true;
}

/*bool SResource::OnPlayerConnect(long playerid)
{
	lua_getglobal(m_lua, "__OnPlayerConnect");
	lua_pushinteger(m_lua, playerid);

	if (lua_pcall(m_lua, 1, 0, 0))
	{
		API::Get().Print("Error in OnPlayerConnect callback");
		API::Get().Print(lua_tostring(m_lua, -1));
	}

	return true;
}*/

char* SResource::OnHTTPRequest(const char* method, const char* url, const char* query, const char* body)
{
	return http(method, url, query, body);
}

bool SResource::OnTick()
{
	tick();
	return true;
}

bool SResource::OnPlayerCommand(long playerid, const char* cmd)
{
	return oncommand(playerid, cmd);
}

void SResource::SetHTTP(const std::function<char*(const char* method, const char* url, const char* query, const char* body)>& t)
{
	http = t;
}

void SResource::SetTick(const std::function<void()>& t)
{
	tick = t;
}

void SResource::SetEvent(const std::function<void(const char* e, std::vector<MValue> *args)>& t)
{
	onevent = t;
}

void SResource::SetCommandProcessor(const std::function<bool(long pid, const char*command)>& t)
{
	oncommand = t;
}


bool SResource::OnKeyStateChanged(long playerid, int keycode, bool isUp)
{
	lua_getglobal(m_lua, "__OnKeyStateChanged");

	lua_pushinteger(m_lua, playerid);
	lua_pushinteger(m_lua, keycode);
	lua_pushboolean(m_lua, isUp);

	if (lua_pcall(m_lua, 3, 0, 0)) API::Get().Print("Error in OnKeyStateChanged callback");

	return true;
}

void SResource::OnEvent(const char* e, std::vector<MValue> *args) {	onevent(e, args); }

SResource::~SResource()
{
}
