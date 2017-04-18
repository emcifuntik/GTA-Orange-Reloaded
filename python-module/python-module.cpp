// python-module.cpp : Definiert die exportierten Funktionen für die DLL-Anwendung.
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
		return "python";
	}

	EXPORT bool OnResourceLoad(const char* resource)
	{
		return pythonFunctions::loadResource(resource);
	}

	EXPORT void OnModuleInit()
	{
		pythonFunctions::init();
		return;
	}

	EXPORT bool OnTick()
	{
		return true;
	}

	EXPORT bool OnPlayerConnect(long playerid)
	{
		return true;
	}

	/*	EXPORT char* OnHTTPRequest(const char* method, const char* url, const char* query, std::string body)
	{
	API::Get().Print("[Python] OnHTTPRequest");
	return ;
	}*/

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
		PyObject *pArgs = PyTuple_New(2);
		PyTuple_SetItem(pArgs, 0, PyLong_FromLong(playerid));
		PyTuple_SetItem(pArgs, 1, PyUnicode_FromString(command));
		pythonFunctions::EventTriggered("PlayerCommand", pArgs);
		//Py_DECREF(pArgs);
		return false;
	}

	EXPORT bool OnPlayerText(long playerid, const char * text)
	{
		PyObject *pArgs = PyTuple_New(2);
		PyTuple_SetItem(pArgs, 0, PyLong_FromLong(playerid));
		PyTuple_SetItem(pArgs, 1, PyUnicode_FromString(text));
		pythonFunctions::EventTriggered("PlayerText", pArgs);
		//Py_DECREF(pArgs);
		return true;
	}

	EXPORT bool OnKeyStateChanged(long playerid, int keycode, bool isUp)
	{
		PyObject *pArgs = PyTuple_New(3);
		PyTuple_SetItem(pArgs, 0, PyLong_FromLong(playerid));
		PyTuple_SetItem(pArgs, 1, PyLong_FromLong(keycode));
		PyTuple_SetItem(pArgs, 2, PyBool_FromLong((int)isUp));
		pythonFunctions::EventTriggered("KeyStateChanged", pArgs);
		//Py_DECREF(pArgs);
		return true;
	}

	EXPORT void OnEvent(const char* e, std::vector<MValue> *args)
	{
		int argsnum = args->size();
		PyObject *pArgs = PyTuple_New(argsnum);
		for (int i = 0; i < argsnum; i++)
		{
			PyObject *pValue;
			MValue param = args->at(i);
			switch (param.type)
			{
			case M_BOOL:
				pValue = PyBool_FromLong(param.getInt());
				break;
			case M_INT:
				pValue = PyLong_FromLong(param.getInt());
				break;
			case M_DOUBLE:
				pValue = PyFloat_FromDouble(param.getDouble());
				break;
			case M_ULONG:
				pValue = PyLong_FromLong(param.getULong());
				break;
			case M_STRING:
				pValue = PyUnicode_FromString(param.getString());
				break;
			}
			PyTuple_SetItem(pArgs, i, pValue);
		}
		pythonFunctions::EventTriggered((char*)e, pArgs);
		//Py_DECREF(pArgs);
		return;
	}
}