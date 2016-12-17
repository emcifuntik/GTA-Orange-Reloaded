#include "stdafx.h"

Python* Python::singleInstance = nullptr;

Python* Python::Get()
{
	if (!singleInstance)
		singleInstance = new Python();
	return singleInstance;
}

bool Python::Connect(const char * script_name)
{
	PyObject* module;
	module = PyImport_ImportModule(script_name);
	if (!module)
		return false;
	else
	{
		pModules.push_back(module);
	}
	return true;
}

void Python::DefineMethods()
{
	Py_InitModule("World", World::Methods);
	Py_InitModule("Player", Player::Methods);
}

Python::Python()
{
	Py_Initialize();
	PyObject *sysPath = PySys_GetObject("path");
	PyObject *path = PyString_FromString("scripts");
	int result = PyList_Insert(sysPath, 0, path);
	DefineMethods();
}

long Python::pCallFunc(char * fName, PyObject* args)
{
	for each (PyObject* module in pModules)
	{
		PyObject *pFunc;
		pFunc = PyObject_GetAttrString(module, fName);
		if (pFunc)
		{
			PyObject * value = PyObject_CallObject(pFunc, args);
			Py_DECREF(pFunc);
			long ret = PyLong_AsLong(value);
			PyErr_Print();
			if (ret != 0)
				return ret;
		}
	}
	return 0;
}
