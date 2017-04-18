#include "stdafx.h"

PyObject* pythonFunctions::GTAOrange_CreateBlipForAll(PyObject* self, PyObject* args)
{
	PyObject *name, *x, *y, *z, *scale, *color, *sprite;
	if (PyArg_UnpackTuple(args, "uffffll", 1, 7, &name, &x, &y, &z, &scale, &color, &sprite))
	{
		unsigned long value = API::Get().CreateBlipForAll(PyUnicode_AsUTF8(name), PyFloat_AsDouble(x), PyFloat_AsDouble(y), PyFloat_AsDouble(z), PyFloat_AsDouble(scale), PyLong_AsLong(color), PyLong_AsLong(sprite));
		return PyLong_FromLong(value);
	}
	return NULL;
}


PyObject* pythonFunctions::GTAOrange_CreateBlipForPlayer(PyObject* self, PyObject* args)
{
	PyObject *playerid, *name, *x, *y, *z, *scale, *color, *sprite;
	if (PyArg_UnpackTuple(args, "luffffll", 1, 8, &playerid, &name, &x, &y, &z, &scale, &color, &sprite))
	{
		unsigned long value = API::Get().CreateBlipForPlayer(PyLong_AsLong(playerid), PyUnicode_AsUTF8(name), PyFloat_AsDouble(x), PyFloat_AsDouble(y), PyFloat_AsDouble(z), PyFloat_AsDouble(scale), PyLong_AsLong(color), PyLong_AsLong(sprite));
		return PyLong_FromLong(value);
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_DeleteBlip(PyObject* self, PyObject* args)
{
	PyObject *blipid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &blipid))
	{
		API::Get().DeleteBlip(PyLong_AsLong(blipid));
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SetBlipColor(PyObject* self, PyObject* args)
{
	PyObject *blipid, *color;
	if (PyArg_UnpackTuple(args, "ll", 1, 2, &blipid, &color))
	{
		API::Get().SetBlipColor(PyLong_AsLong(blipid), PyLong_AsLong(color));
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SetBlipScale(PyObject* self, PyObject* args)
{
	PyObject *blipid, *scale;
	if (PyArg_UnpackTuple(args, "lf", 1, 2, &blipid, &scale))
	{
		API::Get().SetBlipScale(PyLong_AsLong(blipid), PyFloat_AsDouble(scale));
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SetBlipRoute(PyObject* self, PyObject* args)
{
	PyObject *blipid, *route;
	if (PyArg_UnpackTuple(args, "ll", 1, 2, &blipid, &route))
	{
		API::Get().SetBlipRoute(PyLong_AsLong(blipid), (bool)PyLong_AsLong(route));
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SetBlipSprite(PyObject* self, PyObject* args)
{
	PyObject *blipid, *sprite;
	if (PyArg_UnpackTuple(args, "ll", 1, 2, &blipid, &sprite))
	{
		API::Get().SetBlipSprite(PyLong_AsLong(blipid), PyLong_AsLong(sprite));
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SetBlipName(PyObject* self, PyObject* args)
{
	PyObject *blipid, *name;
	if (PyArg_UnpackTuple(args, "lu", 1, 2, &blipid, &name))
	{
		API::Get().SetBlipName(PyLong_AsLong(blipid), PyUnicode_AsUTF8(name));
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SetBlipAsShortRange(PyObject* self, PyObject* args)
{
	PyObject *blipid, *_short;
	if (PyArg_UnpackTuple(args, "ll", 1, 2, &blipid, &_short))
	{
		API::Get().SetBlipAsShortRange(PyLong_AsLong(blipid), (bool)PyLong_AsLong(_short));
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_AttachBlipToPlayer(PyObject* self, PyObject* args)
{
	PyObject *blipid, *playerid;
	if (PyArg_UnpackTuple(args, "ll", 1, 2, &blipid, &playerid))
	{
		API::Get().AttachBlipToPlayer(PyLong_AsLong(blipid), PyLong_AsLong(playerid));
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_AttachBlipToVehicle(PyObject* self, PyObject* args)
{
	PyObject *blipid, *vehid;
	if (PyArg_UnpackTuple(args, "ll", 1, 2, &blipid, &vehid))
	{
		API::Get().AttachBlipToVehicle(PyLong_AsLong(blipid), PyLong_AsLong(vehid));
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}