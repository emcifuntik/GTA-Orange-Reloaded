#include "stdafx.h"

PyObject* pythonFunctions::GTAOrange_CreateVehicle(PyObject* self, PyObject* args)
{
	PyObject *vehhash, *x, *y, *z, *heading;
	if (PyArg_UnpackTuple(args, "|l|uffff", 1, 5, &vehhash, &x, &y, &z, &heading))
	{
		unsigned long value;
		if (PyLong_Check(vehhash))
			value = API::Get().CreateVehicle(PyLong_AsLong(vehhash), PyFloat_AsDouble(x), PyFloat_AsDouble(y), PyFloat_AsDouble(z), PyFloat_AsDouble(heading));
		else if (PyUnicode_Check(vehhash))
			value = API::Get().CreateVehicle(API::Get().Hash(PyUnicode_AsUTF8(vehhash)), PyFloat_AsDouble(x), PyFloat_AsDouble(y), PyFloat_AsDouble(z), PyFloat_AsDouble(heading));
		return PyLong_FromLong(value);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_DeleteVehicle(PyObject* self, PyObject* args)
{
	PyObject *vehid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &vehid))
	{
		bool value = API::Get().DeleteVehicle(PyLong_AsLong(vehid));
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
		bool value = API::Get().SetVehiclePosition(PyLong_AsLong(vehid), PyFloat_AsDouble(x), PyFloat_AsDouble(y), PyFloat_AsDouble(z));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GetVehicleRotation(PyObject* self, PyObject* args)
{
	PyObject *vehid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &vehid))
	{
		CVector3 value = API::Get().GetVehicleRotation(PyLong_AsLong(vehid));
		return Py_BuildValue("fff", value.fX, value.fY, value.fZ);
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_SetVehicleRotation(PyObject* self, PyObject* args)
{
	PyObject *vehid, *x, *y, *z;
	if (PyArg_UnpackTuple(args, "lfff", 1, 4, &vehid, &x, &y, &z))
	{
		bool value = API::Get().SetVehicleRotation(PyLong_AsLong(vehid), PyFloat_AsDouble(x), PyFloat_AsDouble(y), PyFloat_AsDouble(z));
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
		CVector3 value = API::Get().GetVehiclePosition(PyLong_AsLong(vehid));
		return Py_BuildValue("fff", value.fX, value.fY, value.fZ);
	}
	return NULL;
}

PyObject* pythonFunctions::GTAOrange_SetVehicleColours(PyObject* self, PyObject* args)
{
	PyObject *vehid, *color1, *color2;
	if (PyArg_UnpackTuple(args, "lll", 1, 3, &vehid, &color1, &color2))
	{
		bool value = API::Get().SetVehicleColours(PyLong_AsLong(vehid), PyLong_AsLong(color1), PyLong_AsLong(color2));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GetVehicleDriver(PyObject* self, PyObject* args)
{
	PyObject *vehid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &vehid))
	{
		return PyLong_FromLong(API::Get().GetVehicleDriver(PyLong_AsLong(vehid)));
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_VehicleExists(PyObject* self, PyObject* args)
{
	PyObject *vehid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &vehid))
	{
		//bool value = API::Get().VehicleExists(PyLong_AsLong(vehid));
		//if (value)
		//	return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}