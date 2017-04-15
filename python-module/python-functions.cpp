#include "stdafx.h"
#include "python-Functions.h"

struct Events {
	char EventName[128];
	std::vector<PyObject*> Eventslist;
};

static struct PyMethodDef methods[] = {
	//{ "LoadClientScript", pythonFunctions::GTAOrange_LoadClientScript, METH_VARARGS, "" }, //Not Implemented (No Lua Compiler)
	{ "TriggerClientEvent", pythonFunctions::GTAOrange_TriggerClientEvent, METH_VARARGS, "" },
	{ "AddServerEvent", pythonFunctions::GTAOrange_AddServerEvent, METH_VARARGS, "" },
	{ "TriggerServerEvent", pythonFunctions::GTAOrange_TriggerServerEvent, METH_VARARGS, "" },

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
	{ "PlayerExists", pythonFunctions::GTAOrange_PlayerExists, METH_VARARGS, "" }, //Not tested
	{ "VehicleExists", pythonFunctions::GTAOrange_VehicleExists, METH_VARARGS, "" }, //Not tested
	{ "CreateVehicle", pythonFunctions::GTAOrange_CreateVehicle, METH_VARARGS, "" },
	{ "DeleteVehicle", pythonFunctions::GTAOrange_DeleteVehicle, METH_VARARGS, "" },
	{ "SetVehiclePosition", pythonFunctions::GTAOrange_SetVehiclePosition, METH_VARARGS, "" },
	{ "GetVehiclePosition", pythonFunctions::GTAOrange_GetVehiclePosition, METH_VARARGS, "" },
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

API * APIP = nullptr;
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
				args.push_back(PyLong_AsLong(Item));
			else if (PyUnicode_Check(Item))
				args.push_back(PyUnicode_AsUTF8(Item));
			else
				APIP->Print("You can only pass bools, numbers and strings");
		}
		APIP->ClientEvent(PyUnicode_AsUTF8(event), args, PyLong_AsLong(playerid));
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
		APIP->Print(text.c_str());
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

PyObject* pythonFunctions::GTAOrange_SetPlayerPosition(PyObject* self, PyObject* args)
{
	PyObject *playerid, *x, *y, *z;
	if (PyArg_UnpackTuple(args, "lfff", 1, 4, &playerid, &x, &y, &z))
	{
		bool value = APIP->SetPlayerPosition(PyLong_AsLong(playerid), PyFloat_AsDouble(x), PyFloat_AsDouble(y), PyFloat_AsDouble(z));
		if(value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GetPlayerPosition(PyObject* self, PyObject* args)
{
	PyObject *playerid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &playerid))
	{
		CVector3 value = APIP->GetPlayerPosition(PyLong_AsLong(playerid));
		return Py_BuildValue("fff", value.fX, value.fY, value.fZ);
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_IsPlayerInRange(PyObject* self, PyObject* args)
{
	PyObject *playerid, *x, *y, *z, *range;
	if (PyArg_UnpackTuple(args, "lfff", 1, 5, &playerid, &x, &y, &z, &range))
	{
		bool value = APIP->IsPlayerInRange(PyLong_AsLong(playerid), PyFloat_AsDouble(x), PyFloat_AsDouble(y), PyFloat_AsDouble(z), PyFloat_AsDouble(range));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SetPlayerHeading(PyObject* self, PyObject* args)
{
	PyObject *playerid, *angle;
	if (PyArg_UnpackTuple(args, "lf", 1, 2, &playerid, &angle))
	{
		bool value = APIP->SetPlayerHeading(PyLong_AsLong(playerid), PyFloat_AsDouble(angle));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GetPlayerHeading(PyObject* self, PyObject* args)
{
	PyObject *playerid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &playerid))
	{
		float value = APIP->GetPlayerHeading(PyLong_AsLong(playerid));
		return PyFloat_FromDouble(value);
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_RemovePlayerWeapons(PyObject* self, PyObject* args)
{
	PyObject *playerid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &playerid))
	{
		bool value = APIP->RemovePlayerWeapons(PyLong_AsLong(playerid));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GivePlayerWeapon(PyObject* self, PyObject* args)
{
	PyObject *playerid, *weapon, *ammo, *pyString;
	if (PyArg_UnpackTuple(args, "l|l|ul", 1, 3, &playerid, &weapon, &ammo))
	{
		bool value;
		//int number = std::stoi(PyUnicode_AsUTF8(weapon));
		if (PyLong_Check(weapon))
			value = APIP->GivePlayerWeapon(PyLong_AsLong(playerid), PyLong_AsLong(weapon), PyLong_AsLong(ammo));
		else if (PyUnicode_Check(weapon))
			value = APIP->GivePlayerWeapon(PyLong_AsLong(playerid), APIP->Hash(PyUnicode_AsUTF8(weapon)), PyLong_AsLong(ammo));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GivePlayerAmmo(PyObject* self, PyObject* args)
{
	PyObject *playerid, *weapon, *ammo;
	if (PyArg_UnpackTuple(args, "l|l|ul", 1, 3, &playerid, &weapon, &ammo))
	{
		bool value;
		if (PyLong_Check(weapon))
			value = APIP->GivePlayerAmmo(PyLong_AsLong(playerid), PyLong_AsLong(weapon), PyLong_AsLong(ammo));
		else if (PyUnicode_Check(weapon))
			value = APIP->GivePlayerAmmo(PyLong_AsLong(playerid), APIP->Hash(PyUnicode_AsUTF8(weapon)), PyLong_AsLong(ammo));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GivePlayerMoney(PyObject* self, PyObject* args)
{
	PyObject *playerid, *money;
	if (PyArg_UnpackTuple(args, "ll", 1, 2, &playerid, &money))
	{
		bool value = APIP->GivePlayerMoney(PyLong_AsLong(playerid), PyLong_AsLong(money));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SetPlayerMoney(PyObject* self, PyObject* args)
{
	PyObject *playerid, *money;
	if (PyArg_UnpackTuple(args, "ll", 1, 2, &playerid, &money))
	{
		bool value = APIP->SetPlayerMoney(PyLong_AsLong(playerid), PyLong_AsLong(money));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_ResetPlayerMoney(PyObject* self, PyObject* args)
{
	PyObject *playerid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &playerid))
	{
		bool value = APIP->ResetPlayerMoney(PyLong_AsLong(playerid));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GetPlayerMoney(PyObject* self, PyObject* args)
{
	PyObject *playerid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &playerid))
	{
		size_t value = APIP->GetPlayerMoney(PyLong_AsLong(playerid));
		return PyLong_FromSize_t(value);
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_SetPlayerModel(PyObject* self, PyObject* args)
{
	PyObject *playerid, *model;
	if (PyArg_UnpackTuple(args, "ll", 1, 2, &playerid, &model))
	{
		bool value;
		if (PyLong_Check(model))
			value = APIP->SetPlayerModel(PyLong_AsLong(playerid), PyLong_AsLong(model));
		else if (PyUnicode_Check(model))
			value = APIP->SetPlayerModel(PyLong_AsLong(playerid), APIP->Hash(PyUnicode_AsUTF8(model)));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GetPlayerModel(PyObject* self, PyObject* args)
{
	PyObject *playerid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &playerid))
	{
		long value = APIP->GetPlayerModel(PyLong_AsLong(playerid));
		return PyLong_FromLong(value);
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_SetPlayerName(PyObject* self, PyObject* args)
{
	PyObject *playerid, *name;
	if (PyArg_UnpackTuple(args, "lu", 1, 2, &playerid, &name))
	{
		bool value = APIP->SetPlayerName(PyLong_AsLong(playerid), PyUnicode_AsUTF8(name));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GetPlayerName(PyObject* self, PyObject* args)
{
	PyObject *playerid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &playerid))
	{
		std::string value = APIP->GetPlayerName(PyLong_AsLong(playerid));
		return PyUnicode_FromString(value.c_str());
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_SetPlayerHealth(PyObject* self, PyObject* args)
{
	PyObject *playerid, *health;
	if (PyArg_UnpackTuple(args, "lf", 1, 2, &playerid, &health))
	{
		float health2 = PyFloat_AsDouble(health);
		bool value = APIP->SetPlayerHealth(PyLong_AsLong(playerid), health2 == 0 ? 0 : health2 / 2 + 100);
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GetPlayerHealth(PyObject* self, PyObject* args)
{
	PyObject *playerid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &playerid))
	{
		float value = APIP->GetPlayerHealth(PyLong_AsLong(playerid));
		return PyFloat_FromDouble(value);
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_SetPlayerArmour(PyObject* self, PyObject* args)
{
	PyObject *playerid, *armour;
	if (PyArg_UnpackTuple(args, "lf", 1, 2, &playerid, &armour))
	{
		float armour2 = PyFloat_AsDouble(armour);
		bool value = APIP->SetPlayerArmour(PyLong_AsLong(playerid), armour2 == 0 ? 0 : armour2 / 2 + 100);
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GetPlayerArmour(PyObject* self, PyObject* args)
{
	PyObject *playerid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &playerid))
	{
		float value = APIP->GetPlayerArmour(PyLong_AsLong(playerid));
		return PyFloat_FromDouble(value);
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_SetPlayerColor(PyObject* self, PyObject* args)
{
	PyObject *playerid, *color;
	if (PyArg_UnpackTuple(args, "ll", 1, 2, &playerid, &color))
	{
		bool value = APIP->SetPlayerColor(PyLong_AsLong(playerid), PyLong_AsLong(color));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GetPlayerColor(PyObject* self, PyObject* args)
{
	PyObject *playerid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &playerid))
	{
		unsigned int value = APIP->GetPlayerColor(PyLong_AsLong(playerid));
		return PyLong_FromLong(value);
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_BroadcastClientMessage(PyObject* self, PyObject* args)
{
	PyObject *message, *color;
	if (PyArg_UnpackTuple(args, "ul", 1, 2, &message, &color))
	{
		APIP->BroadcastClientMessage(PyUnicode_AsUTF8(message), PyLong_AsLong(color));
		//if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SendClientMessage(PyObject* self, PyObject* args)
{
	PyObject *playerid, *message, *color;
	if (PyArg_UnpackTuple(args, "lul", 1, 3, &playerid, &message, &color))
	{
		bool value = APIP->SendClientMessage(PyLong_AsLong(playerid), PyUnicode_AsUTF8(message), PyLong_AsLong(color));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SetPlayerIntoVehicle(PyObject* self, PyObject* args)
{
	PyObject *playerid, *vehid, *seat;
	if (PyArg_UnpackTuple(args, "lll", 1, 3, &playerid, &vehid, &seat))
	{
		bool value = APIP->SetPlayerIntoVehicle(PyLong_AsLong(playerid), PyLong_AsLong(vehid), (char)PyLong_AsLong(seat));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_DisablePlayerHud(PyObject* self, PyObject* args)
{
	PyObject *playerid, *toggle;
	if (PyArg_UnpackTuple(args, "ll", 1, 2, &playerid, &toggle))
	{
		bool disable = false;
		if (PyObject_IsTrue(toggle) == 1)
			disable = true;
		APIP->DisablePlayerHud(PyLong_AsLong(playerid), disable);
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GetPlayerGUID(PyObject* self, PyObject* args)
{
	PyObject *playerid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &playerid))
	{
		unsigned long value = APIP->GetPlayerGUID(PyLong_AsLong(playerid));
		return PyLong_FromLong(value);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_PlayerExists(PyObject* self, PyObject* args)
{
	PyObject *playerid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &playerid))
	{
		//bool value = APIP->PlayerExists(PyLong_AsLong(playerid));
		//if (value)
		//	return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_VehicleExists(PyObject* self, PyObject* args)
{
	PyObject *vehid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &vehid))
	{
		//bool value = APIP->VehicleExists(PyLong_AsLong(vehid));
		//if (value)
		//	return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_CreateVehicle(PyObject* self, PyObject* args)
{
	PyObject *vehhash, *x, *y, *z, *heading;
	if (PyArg_UnpackTuple(args, "|l|uffff", 1, 5, &vehhash, &x, &y, &z, &heading))
	{
		unsigned long value;
		if (PyLong_Check(vehhash))
			value = APIP->CreateVehicle(PyLong_AsLong(vehhash), PyFloat_AsDouble(x), PyFloat_AsDouble(y), PyFloat_AsDouble(z), PyFloat_AsDouble(heading));
		else if(PyUnicode_Check(vehhash))
			value = APIP->CreateVehicle(APIP->Hash(PyUnicode_AsUTF8(vehhash)), PyFloat_AsDouble(x), PyFloat_AsDouble(y), PyFloat_AsDouble(z), PyFloat_AsDouble(heading));
		return PyLong_FromLong(value);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_DeleteVehicle(PyObject* self, PyObject* args)
{
	PyObject *vehid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &vehid))
	{
		bool value = APIP->DeleteVehicle(PyLong_AsLong(vehid));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SetVehiclePosition(PyObject* self, PyObject* args)
{
	PyObject *vehid, *x, *y, *z;
	if (PyArg_UnpackTuple(args, "lfff", 1, 4, &vehid, &x, &y, &z))
	{
		bool value = APIP->SetVehiclePosition(PyLong_AsLong(vehid), PyFloat_AsDouble(x), PyFloat_AsDouble(y), PyFloat_AsDouble(z));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GetVehiclePosition(PyObject* self, PyObject* args)
{
	PyObject *vehid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &vehid))
	{
		CVector3 value = APIP->GetVehiclePosition(PyLong_AsLong(vehid));
		return Py_BuildValue("fff", value.fX, value.fY, value.fZ);
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_CreateObject(PyObject* self, PyObject* args)
{
	PyObject *model, *x, *y, *z, *pitch, *yaw, *roll;
	if (PyArg_UnpackTuple(args, "lffffff", 1, 7, &model, &x, &y, &z, &pitch, &yaw, &roll))
	{
		unsigned long value = APIP->CreateObject(PyLong_AsLong(model), PyFloat_AsDouble(x), PyFloat_AsDouble(y), PyFloat_AsDouble(z), PyFloat_AsDouble(pitch), PyFloat_AsDouble(yaw), PyFloat_AsDouble(roll));
		return PyLong_FromLong(value);
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_DeleteObject(PyObject* self, PyObject* args)
{
	PyObject *objid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &objid))
	{
		bool value = APIP->DeleteObject(PyLong_AsLong(objid));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_CreatePickup(PyObject* self, PyObject* args)
{
	PyObject *type, *x, *y, *z, *scale;
	if (PyArg_UnpackTuple(args, "lffff", 1, 5, &type, &x, &y, &z, &scale))
	{
		unsigned long value = APIP->CreatePickup(PyLong_AsLong(type), PyFloat_AsDouble(x), PyFloat_AsDouble(y), PyFloat_AsDouble(z), PyFloat_AsDouble(scale));
		return PyLong_FromLong(value);
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_CreateBlipForAll(PyObject* self, PyObject* args)
{
	PyObject *name, *x, *y, *z, *scale, *color, *sprite;
	if (PyArg_UnpackTuple(args, "uffffll", 1, 7, &name, &x, &y, &z, &scale, &color, &sprite))
	{
		unsigned long value = APIP->CreateBlipForAll(PyUnicode_AsUTF8(name), PyFloat_AsDouble(x), PyFloat_AsDouble(y), PyFloat_AsDouble(z), PyFloat_AsDouble(scale), PyLong_AsLong(color), PyLong_AsLong(sprite));
		return PyLong_FromLong(value);
	}
	return NULL;
}


PyObject* pythonFunctions::GTAOrange_CreateBlipForPlayer(PyObject* self, PyObject* args)
{
	PyObject *playerid, *name, *x, *y, *z, *scale, *color, *sprite;
	if (PyArg_UnpackTuple(args, "luffffll", 1, 8, &playerid, &name, &x, &y, &z, &scale, &color, &sprite))
	{
		unsigned long value = APIP->CreateBlipForPlayer(PyLong_AsLong(playerid), PyUnicode_AsUTF8(name), PyFloat_AsDouble(x), PyFloat_AsDouble(y), PyFloat_AsDouble(z), PyFloat_AsDouble(scale), PyLong_AsLong(color), PyLong_AsLong(sprite));
		return PyLong_FromLong(value);
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_DeleteBlip(PyObject* self, PyObject* args)
{
	PyObject *blipid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &blipid))
	{
		APIP->DeleteBlip(PyLong_AsLong(blipid));
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SetBlipColor(PyObject* self, PyObject* args)
{
	PyObject *blipid, *color;
	if (PyArg_UnpackTuple(args, "ll", 1, 2, &blipid, &color))
	{
		APIP->SetBlipColor(PyLong_AsLong(blipid), PyLong_AsLong(color));
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SetBlipScale(PyObject* self, PyObject* args)
{
	PyObject *blipid, *scale;
	if (PyArg_UnpackTuple(args, "lf", 1, 2, &blipid, &scale))
	{
		APIP->SetBlipScale(PyLong_AsLong(blipid), PyFloat_AsDouble(scale));
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SetBlipRoute(PyObject* self, PyObject* args)
{
	PyObject *blipid, *route;
	if (PyArg_UnpackTuple(args, "ll", 1, 2, &blipid, &route))
	{
		APIP->SetBlipRoute(PyLong_AsLong(blipid), (bool)PyLong_AsLong(route));
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SetBlipSprite(PyObject* self, PyObject* args)
{
	PyObject *blipid, *sprite;
	if (PyArg_UnpackTuple(args, "ll", 1, 2, &blipid, &sprite))
	{
		APIP->SetBlipSprite(PyLong_AsLong(blipid), PyLong_AsLong(sprite));
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SetBlipName(PyObject* self, PyObject* args)
{
	PyObject *blipid, *name;
	if (PyArg_UnpackTuple(args, "lu", 1, 2, &blipid, &name))
	{
		APIP->SetBlipName(PyLong_AsLong(blipid), PyUnicode_AsUTF8(name));
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SetBlipAsShortRange(PyObject* self, PyObject* args)
{
	PyObject *blipid, *_short;
	if (PyArg_UnpackTuple(args, "ll", 1, 2, &blipid, &_short))
	{
		APIP->SetBlipAsShortRange(PyLong_AsLong(blipid), (bool)PyLong_AsLong(_short));
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_AttachBlipToPlayer(PyObject* self, PyObject* args)
{
	PyObject *blipid, *playerid;
	if (PyArg_UnpackTuple(args, "ll", 1, 2, &blipid, &playerid))
	{
		APIP->AttachBlipToPlayer(PyLong_AsLong(blipid), PyLong_AsLong(playerid));
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_AttachBlipToVehicle(PyObject* self, PyObject* args)
{
	PyObject *blipid, *vehid;
	if (PyArg_UnpackTuple(args, "ll", 1, 2, &blipid, &vehid))
	{
		APIP->AttachBlipToVehicle(PyLong_AsLong(blipid), PyLong_AsLong(vehid));
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_CreateMarkerForAll(PyObject* self, PyObject* args)
{
	PyObject *x, *y, *z, *height, *radius;
	if (PyArg_UnpackTuple(args, "fffff", 1, 5, &x, &y, &z, &height, &radius))
	{
		unsigned long value = APIP->CreateMarkerForAll(PyFloat_AsDouble(x), PyFloat_AsDouble(y), PyFloat_AsDouble(z), PyFloat_AsDouble(height), PyFloat_AsDouble(radius));
		return PyLong_FromLong(value);
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_CreateMarkerForPlayer(PyObject* self, PyObject* args)
{
	PyObject *playerid, *x, *y, *z, *height, *radius;
	if (PyArg_UnpackTuple(args, "lfffff", 1, 6, &playerid, &x, &y, &z, &height, &radius))
	{
		unsigned long value = APIP->CreateMarkerForPlayer(PyLong_AsLong(playerid), PyFloat_AsDouble(x), PyFloat_AsDouble(y), PyFloat_AsDouble(z), PyFloat_AsDouble(height), PyFloat_AsDouble(radius));
		return PyLong_FromLong(value);
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_DeleteMarker(PyObject* self, PyObject* args)
{
	PyObject *markerid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &markerid))
	{
		APIP->DeleteBlip(PyLong_AsLong(markerid));
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SendNotification(PyObject* self, PyObject* args)
{
	PyObject *playerid, *name;
	if (PyArg_UnpackTuple(args, "lu", 1, 2, &playerid, &name))
	{
		bool value = APIP->SendNotification(PyLong_AsLong(playerid), PyUnicode_AsUTF8(name));
		return PyBool_FromLong(value);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SetInfoMsg(PyObject* self, PyObject* args)
{
	PyObject *playerid, *name;
	if (PyArg_UnpackTuple(args, "lu", 1, 2, &playerid, &name))
	{
		bool value = APIP->SetInfoMsg(PyLong_AsLong(playerid), PyUnicode_AsUTF8(name));
		return PyBool_FromLong(value);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_UnsetInfoMsg(PyObject* self, PyObject* args)
{
	PyObject *playerid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &playerid))
	{
		bool value = APIP->UnsetInfoMsg(PyLong_AsLong(playerid));
		return PyBool_FromLong(value);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_Create3DTextForAll(PyObject* self, PyObject* args)
{
	PyObject *text, *x, *y, *z, *color, *outcolor, *frontsize;
	if (PyArg_UnpackTuple(args, "ufffllf", 1, 7, &text, &x, &y, &z, &color, &outcolor, &frontsize))
	{
		unsigned long value = APIP->Create3DText(PyUnicode_AsUTF8(text), PyFloat_AsDouble(x), PyFloat_AsDouble(y), PyFloat_AsDouble(z), PyLong_AsLong(color), PyLong_AsLong(outcolor), PyFloat_AsDouble(frontsize));
		return PyLong_FromLong(value);
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_Create3DTextForPlayer(PyObject* self, PyObject* args)
{
	PyObject *playerid, *text, *x, *y, *z, *color, *outcolor;
	if (PyArg_UnpackTuple(args, "lufffll", 1, 7, &playerid, &text, &x, &y, &z, &color, &outcolor))
	{
		unsigned long value = APIP->Create3DTextForPlayer(PyLong_AsLong(playerid), PyUnicode_AsUTF8(text), PyFloat_AsDouble(x), PyFloat_AsDouble(y), PyFloat_AsDouble(z), PyLong_AsLong(color), PyLong_AsLong(outcolor));
		return PyLong_FromLong(value);
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_Attach3DTextToPlayer(PyObject* self, PyObject* args)
{
	PyObject *textid, *playerid, *oX, *oY, *oZ;
	if (PyArg_UnpackTuple(args, "llfff", 1, 5, &textid, &playerid, &oX, &oY, &oZ))
	{
		bool value = APIP->Attach3DTextToPlayer(PyLong_AsLong(textid), PyLong_AsLong(playerid), PyFloat_AsDouble(oX), PyFloat_AsDouble(oY), PyFloat_AsDouble(oZ));
		return PyBool_FromLong(value);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_Attach3DTextToVehicle(PyObject* self, PyObject* args)
{
	PyObject *textid, *vehid, *oX, *oY, *oZ;
	if (PyArg_UnpackTuple(args, "llfff", 1, 5, &textid, &vehid, &oX, &oY, &oZ))
	{
		bool value = APIP->Attach3DTextToVehicle(PyLong_AsLong(textid), PyLong_AsLong(vehid), PyFloat_AsDouble(oX), PyFloat_AsDouble(oY), PyFloat_AsDouble(oZ));
		return PyBool_FromLong(value);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_Set3DTextContent(PyObject* self, PyObject* args)
{
	PyObject *textid, *text;
	if (PyArg_UnpackTuple(args, "l", 1, 2, &textid, &text))
	{
		bool value = APIP->Set3DTextContent(PyLong_AsLong(textid), PyUnicode_AsUTF8(text));
		return PyBool_FromLong(value);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_Delete3DText(PyObject* self, PyObject* args)
{
	PyObject *textid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &textid))
	{
		bool value = APIP->Delete3DText(PyLong_AsLong(textid));
		return PyBool_FromLong(value);
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
		APIP->Print(message.c_str());
	}*/
	PyErr_Print();
}

void pythonFunctions::addAPI(API *pAPI)
{
	APIP = pAPI;
	return;
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
		APIP->Print(buffer);
	}
	Py_DECREF(pModule);

	APIP->Print("Python module 0.2 loaded");
	return;
}

bool pythonFunctions::loadResource(const char* resource)
{
	char buffer[34 + sizeof(resource)];
	sprintf(buffer, "[Python] Starting resource %s", resource);
	APIP->Print(buffer);

	PyEval_AcquireThread(Threads);

	PyObject *pName = PyUnicode_DecodeFSDefault(resource);

	PyObject *pModule = PyImport_Import(pName);
	PyEval_ReleaseThread(Threads);
	Py_DECREF(pName);

	if (pModule != NULL) {
		sprintf(buffer, "[Python] Started %s", resource);
		APIP->Print(buffer);
		Py_DECREF(pModule);
		return true;
	}
	else {
		pythonFunctions::PrintError();
		sprintf(buffer, "[Python] Failed to load %s", resource);
		APIP->Print(buffer);
		Py_DECREF(pModule);
		return false;
	}
}

PyObject* pythonFunctions::PyInit_GTAOrange(void)
{
	return PyModule_Create(&modDef);
}