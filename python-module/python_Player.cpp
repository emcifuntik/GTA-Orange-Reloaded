#include "stdafx.h"

PyObject* pythonFunctions::GTAOrange_KickPlayer(PyObject* self, PyObject* args)
{
	PyObject *playerid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &playerid))
	{
		API::Get().KickPlayer(PyLong_AsLong(playerid));
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SetPlayerPosition(PyObject* self, PyObject* args)
{
	PyObject *playerid, *x, *y, *z;
	if (PyArg_UnpackTuple(args, "lfff", 1, 4, &playerid, &x, &y, &z))
	{
		bool value = API::Get().SetPlayerPosition(PyLong_AsLong(playerid), PyFloat_AsDouble(x), PyFloat_AsDouble(y), PyFloat_AsDouble(z));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GetPlayerPosition(PyObject* self, PyObject* args)
{
	PyObject *playerid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &playerid))
	{
		CVector3 value = API::Get().GetPlayerPosition(PyLong_AsLong(playerid));
		return Py_BuildValue("fff", value.fX, value.fY, value.fZ);
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_IsPlayerInRange(PyObject* self, PyObject* args)
{
	PyObject *playerid, *x, *y, *z, *range;
	if (PyArg_UnpackTuple(args, "lfff", 1, 5, &playerid, &x, &y, &z, &range))
	{
		bool value = API::Get().IsPlayerInRange(PyLong_AsLong(playerid), PyFloat_AsDouble(x), PyFloat_AsDouble(y), PyFloat_AsDouble(z), PyFloat_AsDouble(range));
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
		bool value = API::Get().SetPlayerHeading(PyLong_AsLong(playerid), PyFloat_AsDouble(angle));
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
		float value = API::Get().GetPlayerHeading(PyLong_AsLong(playerid));
		return PyFloat_FromDouble(value);
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_RemovePlayerWeapons(PyObject* self, PyObject* args)
{
	PyObject *playerid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &playerid))
	{
		bool value = API::Get().RemovePlayerWeapons(PyLong_AsLong(playerid));
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
			value = API::Get().GivePlayerWeapon(PyLong_AsLong(playerid), PyLong_AsLong(weapon), PyLong_AsLong(ammo));
		else if (PyUnicode_Check(weapon))
			value = API::Get().GivePlayerWeapon(PyLong_AsLong(playerid), API::Get().Hash(PyUnicode_AsUTF8(weapon)), PyLong_AsLong(ammo));
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
			value = API::Get().GivePlayerAmmo(PyLong_AsLong(playerid), PyLong_AsLong(weapon), PyLong_AsLong(ammo));
		else if (PyUnicode_Check(weapon))
			value = API::Get().GivePlayerAmmo(PyLong_AsLong(playerid), API::Get().Hash(PyUnicode_AsUTF8(weapon)), PyLong_AsLong(ammo));
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
		bool value = API::Get().GivePlayerMoney(PyLong_AsLong(playerid), PyLong_AsLong(money));
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
		bool value = API::Get().SetPlayerMoney(PyLong_AsLong(playerid), PyLong_AsLong(money));
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
		bool value = API::Get().ResetPlayerMoney(PyLong_AsLong(playerid));
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
		size_t value = API::Get().GetPlayerMoney(PyLong_AsLong(playerid));
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
			value = API::Get().SetPlayerModel(PyLong_AsLong(playerid), PyLong_AsLong(model));
		else if (PyUnicode_Check(model))
			value = API::Get().SetPlayerModel(PyLong_AsLong(playerid), API::Get().Hash(PyUnicode_AsUTF8(model)));
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
		long value = API::Get().GetPlayerModel(PyLong_AsLong(playerid));
		return PyLong_FromLong(value);
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_SetPlayerName(PyObject* self, PyObject* args)
{
	PyObject *playerid, *name;
	if (PyArg_UnpackTuple(args, "lu", 1, 2, &playerid, &name))
	{
		bool value = API::Get().SetPlayerName(PyLong_AsLong(playerid), PyUnicode_AsUTF8(name));
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
		std::string value = API::Get().GetPlayerName(PyLong_AsLong(playerid));
		return PyUnicode_FromString(value.c_str());
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_SetPlayerHealth(PyObject* self, PyObject* args)
{
	PyObject *playerid, *health;
	if (PyArg_UnpackTuple(args, "lf", 1, 2, &playerid, &health))
	{
		bool value = API::Get().SetPlayerHealth(PyLong_AsLong(playerid), PyFloat_AsDouble(health));
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
		float value = API::Get().GetPlayerHealth(PyLong_AsLong(playerid));
		return PyFloat_FromDouble(value);
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_SetPlayerArmour(PyObject* self, PyObject* args)
{
	PyObject *playerid, *armour;
	if (PyArg_UnpackTuple(args, "lf", 1, 2, &playerid, &armour))
	{
		bool value = API::Get().SetPlayerArmour(PyLong_AsLong(playerid), PyFloat_AsDouble(armour));
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
		float value = API::Get().GetPlayerArmour(PyLong_AsLong(playerid));
		return PyFloat_FromDouble(value);
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_SetPlayerColor(PyObject* self, PyObject* args)
{
	PyObject *playerid, *color;
	if (PyArg_UnpackTuple(args, "ll", 1, 2, &playerid, &color))
	{
		bool value = API::Get().SetPlayerColor(PyLong_AsLong(playerid), PyLong_AsLong(color));
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
		unsigned int value = API::Get().GetPlayerColor(PyLong_AsLong(playerid));
		return PyLong_FromLong(value);
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_SendClientMessage(PyObject* self, PyObject* args)
{
	PyObject *playerid, *message, *color;
	if (PyArg_UnpackTuple(args, "lul", 1, 3, &playerid, &message, &color))
	{
		bool value = API::Get().SendClientMessage(PyLong_AsLong(playerid), PyUnicode_AsUTF8(message), PyLong_AsLong(color));
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
		bool value = API::Get().SetPlayerIntoVehicle(PyLong_AsLong(playerid), PyLong_AsLong(vehid), (char)PyLong_AsLong(seat));
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
		API::Get().DisablePlayerHud(PyLong_AsLong(playerid), disable);
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GetPlayerGUID(PyObject* self, PyObject* args)
{
	PyObject *playerid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &playerid))
	{
		unsigned long value = API::Get().GetPlayerGUID(PyLong_AsLong(playerid));
		return PyLong_FromLong(value);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SendNotification(PyObject* self, PyObject* args)
{
	PyObject *playerid, *name;
	if (PyArg_UnpackTuple(args, "lu", 1, 2, &playerid, &name))
	{
		bool value = API::Get().SendNotification(PyLong_AsLong(playerid), PyUnicode_AsUTF8(name));
		return PyBool_FromLong(value);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SetInfoMsg(PyObject* self, PyObject* args)
{
	PyObject *playerid, *name;
	if (PyArg_UnpackTuple(args, "lu", 1, 2, &playerid, &name))
	{
		bool value = API::Get().SetInfoMsg(PyLong_AsLong(playerid), PyUnicode_AsUTF8(name));
		return PyBool_FromLong(value);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_UnsetInfoMsg(PyObject* self, PyObject* args)
{
	PyObject *playerid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &playerid))
	{
		bool value = API::Get().UnsetInfoMsg(PyLong_AsLong(playerid));
		return PyBool_FromLong(value);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_PlayerExists(PyObject* self, PyObject* args)
{
	PyObject *playerid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &playerid))
	{
		//bool value = API::Get().PlayerExists(PyLong_AsLong(playerid));
		//if (value)
		//	return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}