#include "stdafx.h"
#include "LuaDefs.h"

CScriptEngine *CScriptEngine::singleInstance = nullptr;

CScriptEngine::CScriptEngine()
{
	m_lua = luaL_newstate();
	luaJIT_setmode(m_lua, 0, LUAJIT_MODE_ENGINE | true);

	luaL_openlibs(m_lua);

	lua_getglobal(m_lua, "_G");
	luaL_setfuncs(m_lua, gfunclib, 0);
	lua_pop(m_lua, 1);

	/*lua_newtable(m_lua);
	luaL_setfuncs(m_lua, mfunclib, 0);
	lua_setglobal(m_lua, "__orange__");*/
}

CScriptEngine::~CScriptEngine()
{

}

CScriptEngine * CScriptEngine::Get()
{
	if (!singleInstance)
		singleInstance = new CScriptEngine();
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

void CScriptEngine::Tick()
{

}
