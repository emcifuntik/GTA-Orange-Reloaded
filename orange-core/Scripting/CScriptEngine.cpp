#include "stdafx.h"
#include "LuaDefs.h"
#include "NativesLua.h"
#include "ClientsideAPILua.h"

CScriptEngine *CScriptEngine::singleInstance = nullptr;

static const struct luaL_Reg gfunclib[] = {
	{ "print", lua_print },
	{ "__setTickHandler", lua_tick },
	{ "__setEventHandler", lua_onevent },
	{ "__menu", lua_menu },
	{ "__setMenuOpened", lua_MenuSetOpened },
	{ "__getMenuOpened", lua_MenuGetOpened },
	{ "__trigger", lua_trigger },
	{ "__keyState", lua_KeyState },
	{ "__chatOpened", lua_ChatShown },

	{ "__getAllPlayers", lua_GetAllPlayers},
	{ "__getPlayerPedHandle", lua_GetPlayerHandle },
	{ "__getPlayerName", lua_GetPlayerName },
	{ "__getMyGUID", lua_GetMyGUID },
	{ "__disableHeadDisplay", lua_DisableHead },
	{ "__setHeadDisplay", lua_SetHeadDisplay },

	{ "__getAllVehicles", lua_GetAllVehicles },
	{ "__getVehicleHandle", lua_GetVehicleHandle },

	{ "__getAllObjects", lua_GetAllObjects },
	{ "__getObjectHandle", lua_GetObjectHandle },

	{ "__getAllBlips", lua_GetAllBlips },
	{ "__getBlipHandle", lua_GetBlipHandle },

	{ NULL, NULL }
};

CScriptEngine::CScriptEngine()
{
	m_lua = luaL_newstate();
	luaJIT_setmode(m_lua, 0, LUAJIT_MODE_ENGINE | true);

	luaL_safeopenlibs(m_lua);

	lua_getglobal(m_lua, "_G");
	luaL_setfuncs(m_lua, gfunclib, 0);
	lua_pop(m_lua, 1);

	register_native_funcs(m_lua);
}

void CScriptEngine::Init()
{
	if (initialized) return;
	initialized = true;

	if (luaL_loadbuffer(m_lua, luaJIT_BC_natives, luaJIT_BC_natives_SIZE, NULL) || lua_pcall(m_lua, 0, 0, 0)) {
		log << "Failed to load Native definition file: " << lua_tostring(m_lua, -1) << std::endl;
	}

	if (luaL_loadbuffer(m_lua, luaJIT_BC_ClientsideAPI, luaJIT_BC_ClientsideAPI_SIZE, NULL) || lua_pcall(m_lua, 0, 0, 0)) {
		log << "Failed to load API definition file: " << lua_tostring(m_lua, -1) << std::endl;
	}
}

CScriptEngine::~CScriptEngine()
{
	if (initialized) lua_close(m_lua);
}

CScriptEngine * CScriptEngine::Get()
{
	if (!singleInstance)
	{
		singleInstance = new CScriptEngine();
	}
	return singleInstance;
}

void CScriptEngine::LoadScript(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	Get()->LoadScript(bitStream);
}

void CScriptEngine::LoadScript(RakNet::BitStream *bsIn)
{
	RakString name;
	size_t size;

	bsIn->Read(name);
	bsIn->Read(size);

	log << "Loaded script: " << name.C_String() << std::endl;

	char* _code = new char[size];

	bsIn->ReadAlignedBytes((unsigned char*)_code, size);

	if (luaL_loadbuffer(m_lua, _code, size, NULL) || lua_pcall(m_lua, 0, 0, 0)) {
		log << "[LUA] " << lua_tostring(m_lua, -1) << std::endl;
	}
}

void CScriptEngine::LoadScript(size_t size, char* code, const char* name)
{
	log << "Loaded script: " << name << std::endl;

	if (luaL_loadbuffer(m_lua, code, size, NULL) || lua_pcall(m_lua, 0, 0, 0)) {
		log << "[LUA] " << lua_tostring(m_lua, -1) << std::endl;
	}
}

void CScriptEngine::SetTick(const std::function<void()>& f)
{
	tickHandler = f;
}

void CScriptEngine::SetCustomHead(const std::function<void(unsigned long guid, bool clear, float distance, CVector3 pos)>& f)
{
	customHead = f;
}

void CScriptEngine::SetEvent(const std::function<void(BitStream *bsIn)>& f)
{
	onevent = f;
}

void CScriptEngine::Tick()
{
	if (!initialized) return;

	if(tickHandler) tickHandler();
}

void CScriptEngine::Close()
{
	delete singleInstance;
	singleInstance = nullptr;
}

void CScriptEngine::Trigger(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	Get()->onevent(bitStream);
}
