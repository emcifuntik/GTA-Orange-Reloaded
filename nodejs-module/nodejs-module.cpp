#include "stdafx.h"
#ifdef _WINDOWS
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
char *_strdup(const char *str) {
	size_t len = std::strlen(str);
	char *x = (char *)malloc(len + 1); /* 1 for the null terminator */
	if (!x) return NULL; /* malloc could not allocate memory */
	std::memcpy(x, str, len + 1); /* copy the string into the new buffer */
	return x;
}
#endif

/*#define REFERENCE_MODULE(name) \
  extern "C" void _register_ ## name(void); \
  void (*fp_register_ ## name)(void) = _register_ ## name
REFERENCE_MODULE(orange);
#undef REFERENCE_MODULE*/

API * API::instance = nullptr;
NodeModule* nodeModule = nullptr;

extern "C"
{
	EXPORT bool Validate(API * api)
	{
		API::Set(api);
		return true;
	}

	EXPORT const char* OnResourceTypeRegister()
	{
		return "nodejs";
	}

	EXPORT bool OnResourceLoad(const char* resource)
	{
		//SResource::Get()->Start(resource);
		nodeModule->OnResourceLoad(resource);
		return true;
	}

	EXPORT void OnModuleInit()
	{
		//SResource::Get()->Init();
		nodeModule = new NodeModule();
		nodeModule->Init();
		API::Get().Print("NodeJS module loaded");
	}

	EXPORT bool OnTick()
	{
		nodeModule->OnTick();
		return true;
		//return SResource::Get()->OnTick();
	}

	EXPORT bool OnPlayerConnect(long playerid)
	{
		return true;
	}

	EXPORT char* OnHTTPRequest(const char* method, const char* url, const char* query, std::string body)
	{
		//return SResource::Get()->OnHTTPRequest(method, url, query, body.c_str());;
		return "lol";
	}

	EXPORT bool OnServerCommand(std::string command)
	{
		return true;
	}

	EXPORT bool OnPlayerDisconnect(long playerid, int reason)
	{
		return true;
	}

	EXPORT bool OnPlayerUpdate(long playerid)
	{

		return true;
	}

	EXPORT bool OnPlayerCommand(long playerid, const char * command)
	{
		return nodeModule->OnPlayerCommand(playerid, command);
		//return SResource::Get()->OnPlayerCommand(playerid, command);
		//return true;
	}

	EXPORT bool OnPlayerText(long playerid, const char * text)
	{
		//return SResource::Get()->OnPlayerText(playerid, text);
		return true;
	}

	EXPORT bool OnKeyStateChanged(long playerid, int keycode, bool isUp)
	{
		//return SResource::Get()->OnKeyStateChanged(playerid, keycode, isUp);
		return true;
	}

	EXPORT void OnEvent(const char* e, std::vector<MValue> *args)
	{
		nodeModule->OnEvent(e, args);
		//SResource::Get()->OnEvent(e, args);
		return;
	}
}

