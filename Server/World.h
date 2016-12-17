#pragma once

namespace World
{
	PyObject *printString(PyObject * self, PyObject* args);
	PyObject *hash(PyObject * self, PyObject* args);

	static PyMethodDef Methods[] = {
		{ "printMessage", printString, METH_VARARGS, "Return nothing" },
		{ "hash", hash, METH_VARARGS, "Returns string hash" },
		{ NULL, NULL, 0, NULL }
	};
};
