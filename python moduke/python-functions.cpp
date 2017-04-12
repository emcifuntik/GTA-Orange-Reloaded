#include "stdafx.h"
#include "python-Functions.h"


static struct PyMethodDef methods[] = {
	{ "GivePlayerOrange", pythonFunctions::GTAOrange_GivePlayerOrange, METH_VARARGS, "" },
	{ NULL, NULL, 0, NULL }
};

static struct PyModuleDef modDef = {
	PyModuleDef_HEAD_INIT, "GTAOrange", NULL, -1, methods,
	NULL, NULL, NULL, NULL
};

PyObject* pythonFunctions::GTAOrange_GivePlayerOrange(PyObject* self, PyObject* args)
{
	API::Get().Print("Player 1 gets oranges");

	return PyBool_FromLong(1);
}

PyObject* pythonFunctions::PyInit_GTAOrange(void)
{
	return PyModule_Create(&modDef);
}
