#include "stdafx.h"
#include "LuaDefs.h"
#include "NativesLua.h"
#include "ClientsideAPILua.h"

CScriptEngine *CScriptEngine::singleInstance = nullptr;

static const struct luaL_Reg gfunclib[] = {
	{ "print", lua_print },
	{ "__setTickHandler", lua_tick },
	{ "__menu", lua_menu },
	{ "__trigger", lua_trigger },
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

	char* _code = new char[size];

	bsIn->ReadAlignedBytes((unsigned char*)_code, size);

	log << "Loaded script: " << name.C_String() << std::endl;

	if (luaL_loadbuffer(m_lua, _code, size, NULL) || lua_pcall(m_lua, 0, 0, 0)) {
		log << "[LUA] " << lua_tostring(m_lua, -1) << std::endl;
	}

	log << "success" << std::endl;
}

void CScriptEngine::SetTick(const std::function<void()>& f)
{
	tickHandler = f;
}

void CScriptEngine::Tick()
{
	if (!initialized) return;
	if(tickHandler) tickHandler();
}
