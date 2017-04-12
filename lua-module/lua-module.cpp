// lua-module.cpp: определяет экспортированные функции для приложения DLL.
//

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


API * API::instance = nullptr;

extern "C"
{
	EXPORT bool Validate(API * api)
	{
		API::Set(api);
		return true;
	}

	EXPORT const char* OnResourceTypeRegister()
	{
		return "lua";
	}

	EXPORT bool OnResourceLoad(const char* resource)
	{
		SResource::Get()->Start(resource);
		return true;
	}

	EXPORT void OnModuleInit()
	{
		SResource::Get()->Init();
		API::Get().Print("Lua module loaded");
	}

	EXPORT bool OnTick()
	{
		return SResource::Get()->OnTick();
	}

	EXPORT bool OnPlayerConnect(long playerid)
	{
		return true;
	}

	EXPORT char* OnHTTPRequest(const char* method, const char* url, const char* query, std::string body)
	{
		return SResource::Get()->OnHTTPRequest(method, url, query, body.c_str());;
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
		return SResource::Get()->OnPlayerCommand(playerid, command);
	}

	EXPORT bool OnPlayerText(long playerid, const char * text)
	{
		return SResource::Get()->OnPlayerText(playerid, text);
	}

	EXPORT bool OnKeyStateChanged(long playerid, int keycode, bool isUp)
	{
		return SResource::Get()->OnKeyStateChanged(playerid, keycode, isUp);
	}

	EXPORT void OnEvent(const char* e, std::vector<MValue> *args)
	{
		SResource::Get()->OnEvent(e, args);
	}
}

