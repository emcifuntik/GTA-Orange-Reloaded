#pragma once
class CScriptEngine
{
	static CScriptEngine *singleInstance;
	CScriptEngine();

	lua_State *m_lua;
	std::function<void()> tickHandler;
public:
	~CScriptEngine();
	static CScriptEngine * Get();

	static void LoadScript(RakNet::BitStream * bitStream, RakNet::Packet * packet);
	void LoadScript(RakNet::BitStream * bitStream);
	void SetTick(const std::function<void()>& f);
	void Tick();
};