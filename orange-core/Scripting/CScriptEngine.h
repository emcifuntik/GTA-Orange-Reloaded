#pragma once
class CScriptEngine
{
	static CScriptEngine *singleInstance;
	CScriptEngine();

	lua_State *m_lua;
public:
	~CScriptEngine();
	static CScriptEngine * Get();

	static void LoadScript(RakNet::BitStream * bitStream, RakNet::Packet * packet);
	void LoadScript(RakNet::BitStream * bitStream);
	void Tick();
};