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


	EXPORT void OnModuleInit()
	{
		API::Get().Print("Simple module loaded");
	}

	EXPORT bool OnPlayerConnect(long playerid)
	{
		return true;
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

}

