#include "stdafx.h"

#define luaJIT_BC_main_SIZE 56
static const char luaJIT_BC_main[] = {
	27,76,74,2,2,49,2,0,2,0,2,0,4,54,0,0,0,39,1,1,0,66,0,2,1,75,0,1,0,24,104,105,
	41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,10,112,114,105,110,116,0
};

std::ofstream file("data.luac", std::ios::out | std::ios::binary);
std::stringbuf _code_;
char _code[2591];

static int writer(lua_State *L, const void *p, size_t size, void *u) {

	unsigned int i = 0;

	unsigned char *d = (unsigned char *)p;

	// Print all the bytes on the console.
	/*while (i != size) {
		printf("%c", d[i]);
		++i;
	}*/

	/*if (luaL_loadbuffer(L, (char*)d, size, NULL) || lua_pcall(L, 0, 0, 0)) {
		std::stringstream ss;
		ss << "[LUA] " << lua_tostring(L, -1);
		API::Get().Print(ss.str().c_str());
		return false;
	}*/

	//printf("%c", "call");
	file.write((char*)d, size);
	_code_.sputn((char*)d, size);

	return 0;
}


void compile(lua_State *L, char *file) {
	
	if (luaL_loadfile(L, file) != 0) {
		printf("%s\n", lua_tostring(L, -1));
	}

	lua_dump(L, writer, NULL);
}

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

	/*if (luaL_loadbuffer(m_lua, luaJIT_BC_main, luaJIT_BC_main_SIZE, NULL) || lua_pcall(m_lua, 0, 0, 0)) {
		std::stringstream ss;
		ss << "[LUA] " << lua_tostring(m_lua, -1);
		API::Get().Print(ss.str().c_str());
		return false;
	}*/

	//std::cout << /*lua_dump(m_lua)*/  << std::endl;

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
	
	compile(m_lua, respath);
	lua_pop(m_lua, 1);
	
	/*if (luaL_loadbuffer(m_lua, ((char*)code), csize, NULL) || lua_pcall(m_lua, 0, 0, 0)) {
		std::stringstream ss;
		ss << "[LUA] " << lua_tostring(m_lua, -1);
		API::Get().Print(ss.str().c_str());
		return false;
	}
	
	csize = 0;*/

	/*if (lua_pcall(m_lua, 0, 0, 0)) {
		std::stringstream err;
		err << "[LUA] Could not load main.lua for resource:\n\t" << lua_tostring(m_lua, -1) << name;
		API::Get().Print(err.str().c_str());
		return false;
	}*/


	/*if (luaL_loadfile(m_lua, "modules//lua-module//compiler//run.lua")) {
		std::stringstream err;
		err << "[LUA] " << lua_tostring(m_lua, -1);
		API::Get().Print(err.str().c_str());
		return false;
	}

	lua_pushstring(m_lua, "-b");

	std::stringstream cpath;
	cpath << "../../../resources/" << name << "/main" << ".lua";

	lua_pushstring(m_lua, cpath.str().c_str());

	cpath << "c";

	lua_pushstring(m_lua, cpath.str().c_str());

	if (lua_pcall(m_lua, 3, 0, 0)) {
		std::stringstream err;
		err << "[LUA] " << lua_tostring(m_lua, -1);
		API::Get().Print(err.str().c_str());
		return false;
	}*/

	

	/*if (luaL_loadfile(m_lua, "data.luac") || lua_pcall(m_lua, 0, 0, 0)) {
		std::stringstream ss;
		ss << "[LUA] " << lua_tostring(m_lua, -1);
		API::Get().Print(ss.str().c_str());
		return false;
	}*/

	file.close();

	std::ifstream ffile("data.luac", std::ios::in | std::ios::binary);
	//ffile.read(_code, 2591);
	_code_.sgetn(_code, 2591);

	if (luaL_loadbuffer(m_lua, _code, 2591, NULL) || lua_pcall(m_lua, 0, 0, 0)) {
		std::stringstream ss;
		ss << "[LUAd] " << lua_tostring(m_lua, -1);
		API::Get().Print(ss.str().c_str());
		return false;
	}

	ffile.close();

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
