#pragma once
class SResource
{
public:
	SResource();
	bool Init();
	void AddClientScript(std::string file);
	bool Start(const char * name);
	static SResource *singleInstance;
	static SResource *Get();
	bool OnTick();
	bool OnPlayerCommand(long playerid, const char * cmd);
	void SetHTTP(const std::function<char*(const char* method, const char* url, const char* query, const char* body)>& t);
	void SetTick(const std::function<void()>& t);
	void SetEvent(const std::function<void(const char*e, std::vector<MValue> *args)>& t);
	void SetCommandProcessor(const std::function<bool(long pid, const char* command)>& t);
	char * OnHTTPRequest(const char * method, const char * url, const char * query, const char * body);
	bool OnKeyStateChanged(long playerid, int keycode, bool isUp);
	void OnEvent(const char * e, std::vector<MValue> *args);
	~SResource();
private:
	lua_State *m_lua;
	std::function<void()> tick;
	std::function<char*(const char* method, const char* url, const char* query, const char* body)> http;
	std::function<void(const char* e, std::vector<MValue> *args)> onevent;
	std::function<bool(long pid, const char* command)> oncommand;
};

