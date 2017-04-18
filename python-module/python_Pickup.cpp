#include "stdafx.h"

PyObject* pythonFunctions::GTAOrange_CreatePickup(PyObject* self, PyObject* args)
{
	PyObject *type, *x, *y, *z, *scale;
	if (PyArg_UnpackTuple(args, "lffff", 1, 5, &type, &x, &y, &z, &scale))
	{
		unsigned long value = API::Get().CreatePickup(PyLong_AsLong(type), PyFloat_AsDouble(x), PyFloat_AsDouble(y), PyFloat_AsDouble(z), PyFloat_AsDouble(scale));
		return PyLong_FromLong(value);
	}
	return NULL;
}