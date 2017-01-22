#pragma once
class CScriptEngine
{
	static CScriptEngine *singleInstance;
	CScriptEngine();

	lua_State *m_lua;
	bool initialized;
	std::function<void()> tickHandler;
public:
	~CScriptEngine();
	static CScriptEngine * Get();

	void Init();
	static void LoadScript(RakNet::BitStream * bitStream, RakNet::Packet * packet);
	void LoadScript(RakNet::BitStream * bitStream);
	void SetTick(const std::function<void()>& f);
	lua_State *GetState() { return m_lua; };
	void Tick();
};