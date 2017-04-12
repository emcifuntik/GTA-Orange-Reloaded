// python-module.cpp : Definiert die exportierten Funktionen für die DLL-Anwendung.
//
#include "stdafx.h"
#include "python-Functions.h"

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
		return "python";
	}

	EXPORT bool OnResourceLoad(const char* resource)
	{
		PyObject *pName, *pModule;

		char buffer[28+sizeof(resource)];
		strncpy(buffer, "[Python] Starting resource ", sizeof(buffer));
		strncat(buffer, resource, sizeof(buffer));
		API::Get().Print(buffer);
		
		std::string str = "resources/";
		str.append(resource);
		std::wstring pathWide;
		pathWide.assign(str.begin(), str.end());
		
		PySys_SetPath(pathWide.c_str());
		//pName = PyUnicode_DecodeFSDefault("main");
		/* Error checking of pName left out */

		pModule = PyImport_Import(resource);
		//Py_DECREF(pName);

		if (pModule != NULL) {
			fprintf(stderr, "[Python] Started \"%s\"\n", resource);
			return true;
		}
		else {
			PyErr_Print();
			fprintf(stderr, "[Python] Failed to load \"%s\"\n", resource);
			return false;
		}
	}

	EXPORT void OnModuleInit()
	{
		PyImport_AppendInittab("GTAOrange", &pythonFunctions::PyInit_GTAOrange);
		Py_Initialize();
		API::Get().Print("Python module loaded");
	}

	EXPORT bool OnTick()
	{
		//API::Get().Print("[Python module]: OnTick");
		return true;
	}

	EXPORT bool OnPlayerConnect(long playerid)
	{
		API::Get().Print("[Python] OnPlayerConnect");
		return true;
	}

	/*	EXPORT char* OnHTTPRequest(const char* method, const char* url, const char* query, std::string body)
	{
	API::Get().Print("[Python] OnHTTPRequest");
	return ;
	}*/

	EXPORT bool OnServerCommand(std::string command)
	{
		API::Get().Print("[Python] OnServerCommand");
		return true;
	}

	EXPORT bool OnPlayerDisconnect(long playerid, int reason)
	{
		API::Get().Print("[Python] OnPlayerDisconnect");
		return true;
	}

	EXPORT bool OnPlayerUpdate(long playerid)
	{
		API::Get().Print("[Python] OnPlayerUpdate");
		return true;
	}

	EXPORT bool OnPlayerCommand(long playerid, const char * command)
	{
		API::Get().Print("[Python] OnPlayerCommand");
		return true;
	}

	EXPORT bool OnPlayerText(long playerid, const char * text)
	{
		API::Get().Print("[Python] OnPlayerText");
		return true;
	}

	EXPORT bool OnKeyStateChanged(long playerid, int keycode, bool isUp)
	{
		API::Get().Print("[Python] OnKeyStateChanged");
		return true;
	}

	EXPORT void OnEvent(const char* e, std::vector<MValue> *args)
	{
		API::Get().Print("[Python] OnEvent");
		return;
	}
}