#pragma once
class SResource
{
public:
	SResource();
	bool Init();
	bool Start(const char * name);
	static SResource *singleInstance;
	static SResource *Get();
	bool OnPlayerConnect(long playerid);
	bool OnTick();
	void SetHTTP(const std::function<char*(const char* method, const char* url, const char* query, const char* body)>& t);
	void SetTick(const std::function<void()>& t);
	void SetEvent(const std::function<void(const char*e, std::vector<MValue> *args)>& t);
	char * OnHTTPRequest(const char * method, const char * url, const char * query, const char * body);
	bool OnKeyStateChanged(long playerid, int keycode, bool isUp);
	void OnEvent(const char * e, std::vector<MValue> *args);
	~SResource();
};

