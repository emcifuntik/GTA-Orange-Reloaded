#include "stdafx.h"

PyObject* pythonFunctions::GTAOrange_CreateObject(PyObject* self, PyObject* args)
{
	PyObject *model, *x, *y, *z, *pitch, *yaw, *roll;
	if (PyArg_UnpackTuple(args, "lffffff", 1, 7, &model, &x, &y, &z, &pitch, &yaw, &roll))
	{
		unsigned long value = API::Get().CreateObject(PyLong_AsLong(model), PyFloat_AsDouble(x), PyFloat_AsDouble(y), PyFloat_AsDouble(z), PyFloat_AsDouble(pitch), PyFloat_AsDouble(yaw), PyFloat_AsDouble(roll));
		return PyLong_FromLong(value);
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_DeleteObject(PyObject* self, PyObject* args)
{
	PyObject *objid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &objid))
	{
		bool value = API::Get().DeleteObject(PyLong_AsLong(objid));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}