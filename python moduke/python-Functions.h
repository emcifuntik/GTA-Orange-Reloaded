#pragma once
class pythonFunctions
{
public:
	static PyObject* PyInit_GTAOrange(void);
	static PyObject* GTAOrange_GivePlayerOrange(PyObject* self, PyObject* args);
};

