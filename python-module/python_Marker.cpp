#include "stdafx.h"

PyObject* pythonFunctions::GTAOrange_CreateMarkerForAll(PyObject* self, PyObject* args)
{
	PyObject *x, *y, *z, *height, *radius;
	if (PyArg_UnpackTuple(args, "fffff", 1, 5, &x, &y, &z, &height, &radius))
	{
		unsigned long value = API::Get().CreateMarkerForAll(PyFloat_AsDouble(x), PyFloat_AsDouble(y), PyFloat_AsDouble(z), PyFloat_AsDouble(height), PyFloat_AsDouble(radius));
		return PyLong_FromLong(value);
	}
	return Py_None;
}

PyObject* pythonFunctions::GTAOrange_CreateMarkerForPlayer(PyObject* self, PyObject* args)
{
	PyObject *playerid, *x, *y, *z, *height, *radius;
	if (PyArg_UnpackTuple(args, "lfffff", 1, 6, &playerid, &x, &y, &z, &height, &radius))
	{
		unsigned long value = API::Get().CreateMarkerForPlayer(PyLong_AsLong(playerid), PyFloat_AsDouble(x), PyFloat_AsDouble(y), PyFloat_AsDouble(z), PyFloat_AsDouble(height), PyFloat_AsDouble(radius));
		return PyLong_FromLong(value);
	}
	return Py_None;
}

PyObject* pythonFunctions::GTAOrange_DeleteMarker(PyObject* self, PyObject* args)
{
	PyObject *markerid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &markerid))
	{
		API::Get().DeleteBlip(PyLong_AsLong(markerid));
		return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}