#pragma once
class CScriptEngine
{
	static CScriptEngine *singleInstance;
	CScriptEngine();

	lua_State *m_lua;
	bool initialized;
	std::function<void()> tickHandler;
public:
	std::function<void(BitStream *bsIn)> onevent;
	std::function<void(unsigned long guid, bool clear, float distance, CVector3 pos)> customHead;

	std::vector<std::string> files;

	~CScriptEngine();
	static CScriptEngine * Get();

	void Init();
	static void LoadScript(RakNet::BitStream * bitStream, RakNet::Packet * packet);
	void LoadScript(RakNet::BitStream * bitStream);
	void LoadScript(size_t size, char * code, const char * name);
	void SetTick(const std::function<void()>& f);
	void SetCustomHead(const std::function<void(unsigned long guid, bool clear, float distance, CVector3 pos)>& f);
	void SetEvent(const std::function<void(BitStream*bsIn)>& f);
	lua_State *GetState() { return m_lua; };
	void Tick();
	static void Close();
	static void Trigger(RakNet::BitStream * bitStream, RakNet::Packet * packet);
};