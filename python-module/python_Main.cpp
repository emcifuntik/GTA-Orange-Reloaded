#include "stdafx.h"

struct Events {
	char EventName[128];
	std::vector<PyObject*> Eventslist;
};

static struct PyMethodDef methods[] = {
	//{ "LoadClientScript", pythonFunctions::GTAOrange_LoadClientScript, METH_VARARGS, "" }, //Not Implemented (No Lua Compiler)
	{ "TriggerClientEvent", pythonFunctions::GTAOrange_TriggerClientEvent, METH_VARARGS, "" },
	{ "AddServerEvent", pythonFunctions::GTAOrange_AddServerEvent, METH_VARARGS, "" },
	{ "TriggerServerEvent", pythonFunctions::GTAOrange_TriggerServerEvent, METH_VARARGS, "" },
	{ "KickPlayer", pythonFunctions::GTAOrange_KickPlayer, METH_VARARGS, "" },
	{ "SetPlayerPosition", pythonFunctions::GTAOrange_SetPlayerPosition, METH_VARARGS, "" },	
	{ "GetPlayerPosition", pythonFunctions::GTAOrange_GetPlayerPosition, METH_VARARGS, "" },
	{ "IsPlayerInRange", pythonFunctions::GTAOrange_IsPlayerInRange, METH_VARARGS, "" },
	{ "SetPlayerHeading", pythonFunctions::GTAOrange_SetPlayerHeading, METH_VARARGS, "" },
	{ "GetPlayerHeading", pythonFunctions::GTAOrange_GetPlayerHeading, METH_VARARGS, "" },
	{ "RemovePlayerWeapons", pythonFunctions::GTAOrange_RemovePlayerWeapons, METH_VARARGS, "" },
	{ "GivePlayerWeapon", pythonFunctions::GTAOrange_GivePlayerWeapon, METH_VARARGS, "" },
	{ "GivePlayerAmmo", pythonFunctions::GTAOrange_GivePlayerAmmo, METH_VARARGS, "" },
	{ "GivePlayerMoney", pythonFunctions::GTAOrange_GivePlayerMoney, METH_VARARGS, "" }, //BUG
	{ "SetPlayerMoney", pythonFunctions::GTAOrange_SetPlayerMoney, METH_VARARGS, "" }, //BUG
	{ "ResetPlayerMoney", pythonFunctions::GTAOrange_ResetPlayerMoney, METH_VARARGS, "" }, //BUG
	{ "GetPlayerMoney", pythonFunctions::GTAOrange_GetPlayerMoney, METH_VARARGS, "" }, //BUG
	{ "SetPlayerModel", pythonFunctions::GTAOrange_SetPlayerModel, METH_VARARGS, "" }, 
	{ "GetPlayerModel", pythonFunctions::GTAOrange_GetPlayerModel, METH_VARARGS, "" },
	{ "SetPlayerName", pythonFunctions::GTAOrange_SetPlayerName, METH_VARARGS, "" },
	{ "GetPlayerName", pythonFunctions::GTAOrange_GetPlayerName, METH_VARARGS, "" },
	{ "SetPlayerHealth", pythonFunctions::GTAOrange_SetPlayerHealth, METH_VARARGS, "" }, //Only 200
	{ "GetPlayerHealth", pythonFunctions::GTAOrange_GetPlayerHealth, METH_VARARGS, "" },
	{ "SetPlayerArmour", pythonFunctions::GTAOrange_SetPlayerArmour, METH_VARARGS, "" }, //Only 200
	{ "GetPlayerArmour", pythonFunctions::GTAOrange_GetPlayerArmour, METH_VARARGS, "" },
	{ "SetPlayerColor", pythonFunctions::GTAOrange_SetPlayerColor, METH_VARARGS, "" }, //Bug
	{ "GetPlayerColor", pythonFunctions::GTAOrange_GetPlayerColor, METH_VARARGS, "" }, //Only 0
	{ "BroadcastClientMessage", pythonFunctions::GTAOrange_BroadcastClientMessage, METH_VARARGS, "" },
	{ "SendClientMessage", pythonFunctions::GTAOrange_SendClientMessage, METH_VARARGS, "" },
	{ "SetPlayerIntoVehicle", pythonFunctions::GTAOrange_SetPlayerIntoVehicle, METH_VARARGS, "" }, //Not tested
	{ "DisablePlayerHud", pythonFunctions::GTAOrange_DisablePlayerHud, METH_VARARGS, "" },
	{ "GetPlayerGUID", pythonFunctions::GTAOrange_GetPlayerGUID, METH_VARARGS, "" }, //Not tested
	{ "Print", pythonFunctions::GTAOrange_Print, METH_VARARGS, "" }, // Replaced python function print
	//{ "Hash", pythonFunctions::GTAOrange_Hash, METH_VARARGS, "" },
	{ "PlayerExists", pythonFunctions::GTAOrange_PlayerExists, METH_VARARGS, "" }, //Not Implemented
	{ "VehicleExists", pythonFunctions::GTAOrange_VehicleExists, METH_VARARGS, "" }, //Not Implemented
	{ "CreateVehicle", pythonFunctions::GTAOrange_CreateVehicle, METH_VARARGS, "" },
	{ "DeleteVehicle", pythonFunctions::GTAOrange_DeleteVehicle, METH_VARARGS, "" },
	{ "SetVehiclePosition", pythonFunctions::GTAOrange_SetVehiclePosition, METH_VARARGS, "" },
	{ "GetVehiclePosition", pythonFunctions::GTAOrange_GetVehiclePosition, METH_VARARGS, "" },
	{ "SetVehicleRotation", pythonFunctions::GTAOrange_SetVehicleRotation, METH_VARARGS, "" },
	{ "GetVehicleRotation", pythonFunctions::GTAOrange_GetVehicleRotation, METH_VARARGS, "" },
	{ "SetVehicleColours", pythonFunctions::GTAOrange_SetVehicleColours, METH_VARARGS, "" },
	{ "GetVehicleDriver", pythonFunctions::GTAOrange_GetVehicleDriver, METH_VARARGS, "" },
	{ "CreateObject", pythonFunctions::GTAOrange_CreateObject, METH_VARARGS, "" }, //Not tested
	{ "DeleteObject", pythonFunctions::GTAOrange_DeleteObject, METH_VARARGS, "" }, //Not tested
	{ "CreatePickup", pythonFunctions::GTAOrange_CreatePickup, METH_VARARGS, "" }, //Not tested
	{ "CreateBlipForAll", pythonFunctions::GTAOrange_CreateBlipForAll, METH_VARARGS, "" }, //Not tested
	{ "CreateBlipForPlayer", pythonFunctions::GTAOrange_CreateBlipForPlayer, METH_VARARGS, "" }, //Not tested
	{ "DeleteBlip", pythonFunctions::GTAOrange_DeleteBlip, METH_VARARGS, "" }, //Not tested
	{ "SetBlipColor", pythonFunctions::GTAOrange_SetBlipColor, METH_VARARGS, "" }, //Not tested
	{ "SetBlipScale", pythonFunctions::GTAOrange_SetBlipScale, METH_VARARGS, "" }, //Not tested
	{ "SetBlipRoute", pythonFunctions::GTAOrange_SetBlipRoute, METH_VARARGS, "" }, //Not tested
	{ "SetBlipSprite", pythonFunctions::GTAOrange_SetBlipSprite, METH_VARARGS, "" }, //Not tested
	{ "SetBlipName", pythonFunctions::GTAOrange_SetBlipName, METH_VARARGS, "" }, //Not tested
	{ "SetBlipAsShortRange", pythonFunctions::GTAOrange_SetBlipAsShortRange, METH_VARARGS, "" }, //Not tested
	{ "AttachBlipToPlayer", pythonFunctions::GTAOrange_AttachBlipToPlayer, METH_VARARGS, "" }, //Not tested
	{ "AttachBlipToVehicle", pythonFunctions::GTAOrange_AttachBlipToVehicle, METH_VARARGS, "" }, //Not tested
	{ "CreateMarkerForAll", pythonFunctions::GTAOrange_CreateMarkerForAll, METH_VARARGS, "" }, //Not tested
	{ "CreateMarkerForPlayer", pythonFunctions::GTAOrange_CreateMarkerForPlayer, METH_VARARGS, "" }, //Not tested
	{ "DeleteMarker", pythonFunctions::GTAOrange_DeleteMarker, METH_VARARGS, "" }, //Not tested
	{ "SendNotification", pythonFunctions::GTAOrange_SendNotification, METH_VARARGS, "" }, //Not tested
	{ "SetInfoMsg", pythonFunctions::GTAOrange_SetInfoMsg, METH_VARARGS, "" }, //Not tested
	{ "UnsetInfoMsg", pythonFunctions::GTAOrange_UnsetInfoMsg, METH_VARARGS, "" }, //Not tested
	{ "Create3DTextForAll", pythonFunctions::GTAOrange_Create3DTextForAll, METH_VARARGS, "" }, //Not tested
	{ "Create3DTextForPlayer", pythonFunctions::GTAOrange_Create3DTextForPlayer, METH_VARARGS, "" }, //Not tested
	{ "Attach3DTextToVehicle", pythonFunctions::GTAOrange_Attach3DTextToVehicle, METH_VARARGS, "" }, //Not tested
	{ "Attach3DTextToPlayer", pythonFunctions::GTAOrange_Attach3DTextToPlayer, METH_VARARGS, "" }, //Not tested
	{ "Set3DTextContent", pythonFunctions::GTAOrange_Set3DTextContent, METH_VARARGS, "" }, //Not tested
	{ "Delete3DText", pythonFunctions::GTAOrange_Delete3DText, METH_VARARGS, "" }, //Not tested
	{ NULL, NULL, 0, NULL }
};

static struct PyModuleDef modDef = {
	PyModuleDef_HEAD_INIT, "__orange__", NULL, -1, methods,
	NULL, NULL, NULL, NULL
};

static PyThreadState *Threads;
static std::vector<Events> EventVector;

/*PyObject* pythonFunctions::GTAOrange_LoadClientScript(PyObject* self, PyObject* args)
{
	return PyBool_FromLong(0);
}*/

PyObject* pythonFunctions::GTAOrange_TriggerClientEvent(PyObject* self, PyObject* args)
{
	PyObject *playerid, *event, *parameter;
	if (PyArg_UnpackTuple(args, "lu", 1, 3, &playerid, &event, &parameter))
	{
		int argsnum = PyList_Size(parameter);
		std::vector<MValue> args;
		for (int i = 0; i < argsnum; i++)
		{
			PyObject* Item = PyList_GetItem(parameter, i);
			if (PyBool_Check(Item))
			{
				bool val = false;
				if(PyObject_IsTrue(Item) == 1)
					val = true;
				args.push_back(val);
			}
			else if (PyLong_Check(Item))
				args.push_back((int)PyLong_AsLong(Item));
			else if (PyFloat_Check(Item))
				args.push_back(PyFloat_AsDouble(Item));
			else if (PyUnicode_Check(Item))
				args.push_back(PyUnicode_AsUTF8(Item));
			else
				API::Get().Print("You can only pass bools, numbers and strings");
		}
		API::Get().ClientEvent(PyUnicode_AsUTF8(event), args, PyLong_AsLong(playerid));
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_Print(PyObject* self, PyObject* args)
{
	PyObject *message, *pyString;
	if (PyArg_UnpackTuple(args, "u", 1, 1, &message))
	{
		std::string text;
		if ((pyString = PyObject_Str(message)) != NULL && (PyUnicode_Check(pyString)))
			text = PyUnicode_AsUTF8(pyString);
		else
			return PyBool_FromLong(0);
		API::Get().Print(text.c_str());
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_BroadcastClientMessage(PyObject* self, PyObject* args)
{
	PyObject *message, *color;
	if (PyArg_UnpackTuple(args, "ul", 1, 2, &message, &color))
	{
		API::Get().BroadcastClientMessage(PyUnicode_AsUTF8(message), PyLong_AsLong(color));
		//if (value)
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_AddServerEvent(PyObject* self, PyObject* args)
{
	PyObject *function, *event;
	if (PyArg_UnpackTuple(args, "Su", 1, 2, &function, &event))
	{
		char EventName[128];
		strcpy(EventName, PyUnicode_AsUTF8(event));
		for (std::vector<Events>::iterator iter = EventVector.begin(); iter != EventVector.end(); iter++)
		{
			if (!strcmp(EventName, (*iter).EventName)) 
			{
				(*iter).Eventslist.push_back(function);
				return PyBool_FromLong(1);
			}
		}
		std::vector<PyObject *> newVector;
		newVector.push_back(function);
		struct Events newEvent;
		strcpy(newEvent.EventName, EventName);
		newEvent.Eventslist = newVector;
		EventVector.push_back(newEvent);
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_TriggerServerEvent(PyObject* self, PyObject* args)
{
	PyObject *event, *parameter;
	if (PyArg_UnpackTuple(args, "", 1, 2, &event, &parameter))
	{
		int argsnum = PyList_Size(parameter);
		PyObject *pArgs = PyTuple_New(argsnum);
		for (int i = 0; i < argsnum; i++)
			PyTuple_SetItem(pArgs, i, PyList_GetItem(parameter, i));
		EventTriggered(PyUnicode_AsUTF8(event), pArgs);
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

void pythonFunctions::EventTriggered(char *event, PyObject *args)
{
	for (std::vector<Events>::iterator iter = EventVector.begin(); iter != EventVector.end(); iter++)
	{
		if (!strcmp(event, (*iter).EventName))
		{
			for (std::vector<PyObject*>::iterator iiter = (*iter).Eventslist.begin(); iiter != (*iter).Eventslist.end(); iiter++)
			{
				PyEval_AcquireThread(Threads);
				PyObject_CallObject((*iiter), args);
				Py_DECREF(args);
				if (PyErr_Occurred)
				{

					PrintError();
				}
				PyEval_ReleaseThread(Threads);				
			}
			break;
		}
	}
	return;
}

void pythonFunctions::PrintError()
{
	/*int noerror = 0;
	PyObject *type = NULL, *value = NULL, *traceback = NULL, *pyString = NULL;
	PyErr_Fetch(&type, &value, &traceback);
	PyErr_Clear();
	std::string message = "[Python] Error";

	PyTracebackObject *traceback2 = (PyTracebackObject*)traceback;
	if (traceback != NULL && PyTraceBack_Check(traceback)) {
		message += "\nTraceback (most recent call last):\n";
	}
	if (type != NULL && (pyString = PyObject_Str(type)) != NULL && (PyUnicode_Check(pyString)))
		message += PyUnicode_AsUTF8(pyString);
	else {
		noerror++;
		message += "<unknown exception type> ";
	}
	Py_XDECREF(pyString);
	if (value != NULL && (pyString = PyObject_Str(value)) != NULL && (PyUnicode_Check(pyString))) {
		message += ": ";
		message += PyUnicode_AsUTF8(pyString);
	}
	else {
		noerror++;
		message += "<unknown exception date> ";
	}
	Py_XDECREF(pyString);
	
	Py_XDECREF(type);
	Py_XDECREF(value);
	Py_XDECREF(traceback);
	if (noerror != 2)
	{
		API::Get().Print(message.c_str());
	}*/
	PyErr_Print();
}

void pythonFunctions::init()
{
	PyImport_AppendInittab("__orange__", &pythonFunctions::PyInit_GTAOrange);
	Py_Initialize();
	PyEval_InitThreads();

	char buffer[40];
	sprintf(buffer, "resources");

	PyObject *sys_path, *path;
	sys_path = PySys_GetObject("path");
	path = PyUnicode_FromString(buffer);
	PyList_Append(sys_path, path);
	Py_DECREF(path);

	sprintf(buffer, "modules/python-module");

	path = PyUnicode_FromString(buffer);
	PyList_Append(sys_path, path);
	Py_DECREF(path);

	PyObject *pName = PyUnicode_DecodeFSDefault("GTAOrange");

	PyObject *pModule = PyImport_Import(pName);
	Py_DECREF(pName);

	Threads = PyThreadState_Get();
	PyEval_ReleaseThread(Threads);

	if (pModule == NULL) {
		//pythonFunctions::PrintError();
		sprintf(buffer, "[Python] Error: Wrapper not loaded");
		API::Get().Print(buffer);
	}
	Py_DECREF(pModule);

	API::Get().Print("Python module 0.2 loaded");
	return;
}

bool pythonFunctions::loadResource(const char* resource)
{
	char buffer[34 + sizeof(resource)];
	sprintf(buffer, "[Python] Starting resource %s", resource);
	API::Get().Print(buffer);

	PyEval_AcquireThread(Threads);

	PyObject *pName = PyUnicode_DecodeFSDefault(resource);

	PyObject *pModule = PyImport_Import(pName);
	PyEval_ReleaseThread(Threads);
	Py_DECREF(pName);

	if (pModule != NULL) {
		sprintf(buffer, "[Python] Started %s", resource);
		API::Get().Print(buffer);
		Py_DECREF(pModule);
		return true;
	}
	else {
		pythonFunctions::PrintError();
		sprintf(buffer, "[Python] Failed to load %s", resource);
		API::Get().Print(buffer);
		Py_DECREF(pModule);
		return false;
	}
}

PyObject* pythonFunctions::PyInit_GTAOrange(void)
{
	return PyModule_Create(&modDef);
}