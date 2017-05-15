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
	return Py_None;
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
	return Py_None;
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

PyObject* pythonFunctions::GTAOrange_GetVehicleColours(PyObject* self, PyObject* args)
{
	PyObject *vehid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &vehid))
	{
		int color1, color2;
		bool value = API::Get().GetVehicleColours(PyLong_AsLong(vehid), &color1, &color2);
		if (value)
			return Py_BuildValue("ii", color1, color2);
	}
	return Py_None;
}

PyObject* pythonFunctions::GTAOrange_SetVehicleTyresBulletproof(PyObject* self, PyObject* args)
{
	PyObject *vehid, *bulletproof;
	if (PyArg_UnpackTuple(args, "lb", 1, 2, &vehid, &bulletproof))
	{
		bool state = false;
		if (PyObject_IsTrue(bulletproof) == 1)
			state = true;
		bool value = API::Get().SetVehicleTyresBulletproof(PyLong_AsLong(vehid), state);
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GetVehicleTyresBulletproof(PyObject* self, PyObject* args)
{
	PyObject *vehid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &vehid))
	{
		bool value = API::Get().GetVehicleTyresBulletproof(PyLong_AsLong(vehid));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SetVehicleCustomPrimaryColor(PyObject* self, PyObject* args)
{
	PyObject *vehid, *rColor, *gColor, *bColor;
	if (PyArg_UnpackTuple(args, "llll", 1, 4, &vehid, &rColor, &gColor, &bColor))
	{
		bool value = API::Get().SetVehicleCustomPrimaryColor(PyLong_AsLong(vehid), PyLong_AsLong(rColor), PyLong_AsLong(gColor), PyLong_AsLong(bColor));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GetVehicleCustomPrimaryColor(PyObject* self, PyObject* args)
{
	PyObject *vehid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &vehid))
	{
		int rColor, gColor, bColor;
		bool value = API::Get().GetVehicleCustomPrimaryColor(PyLong_AsLong(vehid), &rColor, &gColor, &bColor);
		if (value)
			return Py_BuildValue("iii", rColor, gColor, bColor);
	}
	return Py_None;
}

PyObject* pythonFunctions::GTAOrange_SetVehicleCustomSecondaryColor(PyObject* self, PyObject* args)
{
	PyObject *vehid, *rColor, *gColor, *bColor;
	if (PyArg_UnpackTuple(args, "llll", 1, 4, &vehid, &rColor, &gColor, &bColor))
	{
		bool value = API::Get().SetVehicleCustomSecondaryColor(PyLong_AsLong(vehid), PyLong_AsLong(rColor), PyLong_AsLong(gColor), PyLong_AsLong(bColor));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GetVehicleCustomSecondaryColor(PyObject* self, PyObject* args)
{
	PyObject *vehid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &vehid))
	{
		int rColor, gColor, bColor;
		bool value = API::Get().GetVehicleCustomSecondaryColor(PyLong_AsLong(vehid), &rColor, &gColor, &bColor);
		if (value)
			return Py_BuildValue("iii", rColor, gColor, bColor);
	}
	return Py_None;
}

PyObject* pythonFunctions::GTAOrange_SetVehicleEngineStatus(PyObject* self, PyObject* args)
{
	PyObject *vehid, *status;
	if (PyArg_UnpackTuple(args, "lb", 1, 2, &vehid, &status))
	{
		bool state = false;
		if (PyObject_IsTrue(status) == 1)
			state = true;
		bool value = API::Get().SetVehicleEngineStatus(PyLong_AsLong(vehid), state, true);
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GetVehicleEngineStatus(PyObject* self, PyObject* args)
{
	PyObject *vehid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &vehid))
	{
		bool value = API::Get().GetVehicleEngineStatus(PyLong_AsLong(vehid));
		if (value)
			return PyBool_FromLong((int)value);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SetVehicleBodyHealth(PyObject* self, PyObject* args)
{
	PyObject *vehid, *health;
	if (PyArg_UnpackTuple(args, "lf", 1, 2, &vehid, &health))
	{
		bool value = API::Get().SetVehicleBodyHealth(PyLong_AsLong(vehid), PyFloat_AsDouble(health));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SetVehicleEngineHealth(PyObject* self, PyObject* args)
{
	PyObject *vehid, *health;
	if (PyArg_UnpackTuple(args, "lf", 1, 2, &vehid, &health))
	{
		bool value = API::Get().SetVehicleEngineHealth(PyLong_AsLong(vehid), PyFloat_AsDouble(health));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SetVehicleTankHealth(PyObject* self, PyObject* args)
{
	PyObject *vehid, *health;
	if (PyArg_UnpackTuple(args, "lf", 1, 2, &vehid, &health))
	{
		bool value = API::Get().SetVehicleTankHealth(PyLong_AsLong(vehid), PyFloat_AsDouble(health));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GetVehicleHealth(PyObject* self, PyObject* args)
{
	PyObject *vehid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &vehid))
	{
		float body, engine, tank;
		bool value = API::Get().GetVehicleHealth(PyLong_AsLong(vehid), &body, &engine, &tank);
		if (value)
			return Py_BuildValue("fff", body, engine, tank);
	}
	return Py_None;
}

PyObject* pythonFunctions::GTAOrange_SetVehicleNumberPlate(PyObject* self, PyObject* args)
{
	PyObject *vehid, *text;
	if (PyArg_UnpackTuple(args, "lu", 1, 2, &vehid, &text))
	{
		bool value = API::Get().SetVehicleNumberPlate(PyLong_AsLong(vehid), PyUnicode_AsUTF8(text));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GetVehicleNumberPlate(PyObject* self, PyObject* args)
{
	PyObject *vehid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &vehid))
	{
		std::string value = API::Get().GetVehicleNumberPlate(PyLong_AsLong(vehid));
		return PyUnicode_FromString(value.c_str());
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SetVehicleNumberPlateStyle(PyObject* self, PyObject* args)
{
	PyObject *vehid, *style;
	if (PyArg_UnpackTuple(args, "ll", 1, 2, &vehid, &style))
	{
		bool value = API::Get().SetVehicleNumberPlateStyle(PyLong_AsLong(vehid), PyLong_AsLong(style));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GetVehicleNumberPlateStyle(PyObject* self, PyObject* args)
{
	PyObject *vehid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &vehid))
	{
		return PyLong_FromLong(API::Get().GetVehicleNumberPlateStyle(PyLong_AsLong(vehid)));
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SetVehicleSirenState(PyObject* self, PyObject* args)
{
	PyObject *vehid, *status;
	if (PyArg_UnpackTuple(args, "lb", 1, 2, &vehid, &status))
	{
		bool state = false;
		if (PyObject_IsTrue(status) == 1)
			state = true;
		bool value = API::Get().SetVehicleSirenState(PyLong_AsLong(vehid), state);
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GetVehicleSirenState(PyObject* self, PyObject* args)
{
	PyObject *vehid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &vehid))
	{
		bool value = API::Get().GetVehicleSirenState(PyLong_AsLong(vehid));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SetVehicleWheelColor(PyObject* self, PyObject* args)
{
	PyObject *vehid, *color;
	if (PyArg_UnpackTuple(args, "ll", 1, 2, &vehid, &color))
	{
		bool value = API::Get().SetVehicleWheelColor(PyLong_AsLong(vehid), PyLong_AsLong(color));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GetVehicleWheelColor(PyObject* self, PyObject* args)
{
	PyObject *vehid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &vehid))
	{
		return PyLong_FromLong(API::Get().GetVehicleWheelColor(PyLong_AsLong(vehid)));
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_SetVehicleWheelType(PyObject* self, PyObject* args)
{
	PyObject *vehid, *type;
	if (PyArg_UnpackTuple(args, "ll", 1, 2, &vehid, &type))
	{
		bool value = API::Get().SetVehicleWheelType(PyLong_AsLong(vehid), PyLong_AsLong(type));
		if (value)
			return PyBool_FromLong(1);
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GetVehicleWheelType(PyObject* self, PyObject* args)
{
	PyObject *vehid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &vehid))
	{
		return PyLong_FromLong(API::Get().GetVehicleWheelType(PyLong_AsLong(vehid)));
	}
	return PyBool_FromLong(0);
}

PyObject* pythonFunctions::GTAOrange_GetVehicleDriver(PyObject* self, PyObject* args)
{
	PyObject *vehid;
	PyArg_UnpackTuple(args, "l", 1, 1, &vehid);
	long result = API::Get().GetVehicleDriver(PyLong_AsLong(vehid));
	if (result != -1)
		return PyLong_FromLong(result);
	return Py_None;
}

PyObject* pythonFunctions::GTAOrange_GetVehiclePassengers(PyObject* self, PyObject* args)
{
	PyObject *vehid;
	if (PyArg_UnpackTuple(args, "l", 1, 1, &vehid))
	{
		std::vector<unsigned int> value = API::Get().GetVehiclePassengers(PyLong_AsLong(vehid));
		if (value.size() == 0)
			return Py_None;
		else if(value.size() == 1)
			return PyLong_FromLong(value.back());
		PyObject *pArgs = PyTuple_New(value.size());
		for (int i = 0; i < value.size(); i++)
		{
			PyTuple_SetItem(pArgs, i, PyLong_FromLong(value.back()));
			value.pop_back();
		}
		return pArgs;
	}
	return Py_None;
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