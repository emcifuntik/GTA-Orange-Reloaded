#include "stdafx.h"

PyObject* pythonFunctions::GTAOrange_Create3DTextForAll(PyObject* self, PyObject* args)
{
	PyObject *text, *x, *y, *z, *color, *outcolor, *frontsize;
	if (PyArg_UnpackTuple(args, "ufffllf", 1, 7, &text, &x, &y, &z, &color, &outcolor, &frontsize))
	{
		unsigned long value = API::Get().Create3DText(PyUnicode_AsUTF8(text), PyFloat_AsDouble(x), PyFloat_AsDouble(y), PyFloat_AsDouble(z), PyLong_AsLong(color), PyLong_AsLong(outcolor), PyFloat_AsDouble(frontsize));
		return PyLong_FromLong(value);
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_Create3DTextForPlayer(PyObject* self, PyObject* args)
{
	PyObject *playerid, *text, *x, *y, *z, *color, *outcolor;
	if (PyArg_UnpackTuple(args, "lufffll", 1, 7, &playerid, &text, &x, &y, &z, &color, &outcolor))
	{
		unsigned long value = API::Get().Create3DTextForPlayer(PyLong_AsLong(playerid), PyUnicode_AsUTF8(text), PyFloat_AsDouble(x), PyFloat_AsDouble(y), PyFloat_AsDouble(z), PyLong_AsLong(color), PyLong_AsLong(outcolor));
		return PyLong_FromLong(value);
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_Attach3DTextToPlayer(PyObject* self, PyObject* args)
{
	PyObject *textid, *playerid, *oX, *oY, *oZ;
	if (PyArg_UnpackTuple(args, "llfff", 1, 5, &textid, &playerid, &oX, &oY, &oZ))
	{
		bool value = API::Get().Attach3DTextToPlayer(PyLong_AsLong(textid), PyLong_AsLong(playerid), PyFloat_AsDouble(oX), PyFloat_AsDouble(oY), PyFloat_AsDouble(oZ));
		return PyBool_FromLong(value);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_Attach3DTextToVehicle(PyObject* self, PyObject* args)
{
	PyObject *textid, *vehid, *oX, *oY, *oZ;
	if (PyArg_UnpackTuple(args, "llfff", 1, 5, &textid, &vehid, &oX, &oY, &oZ))
	{
		bool value = API::Get().Attach3DTextToVehicle(PyLong_AsLong(textid), PyLong_AsLong(vehid), PyFloat_AsDouble(oX), PyFloat_AsDouble(oY), PyFloat_AsDouble(oZ));
		return PyBool_FromLong(value);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_Set3DTextContent(PyObject* self, PyObject* args)
{
	PyObject *textid, *text;
	if (PyArg_UnpackTuple(args, "l", 1, 2, &textid, &text))
	{
		bool value = API::Get().Set3DTextContent(PyLong_AsLong(textid), PyUnicode_AsUTF8(text));
		return PyBool_FromLong(value);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_Delete3DText(PyObject* self, PyObject* args)
{
	PyObject *textid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &textid))
	{
		bool value = API::Get().Delete3DText(PyLong_AsLong(textid));
		return PyBool_FromLong(value);
	}
	return PyBool_FromLong(0);
}