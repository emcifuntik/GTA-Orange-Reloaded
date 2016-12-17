#include "stdafx.h"

namespace World
{
	PyObject *printString(PyObject * self, PyObject* args)
	{
		const char * toPrint;
		if (!PyArg_ParseTuple(args, "s", &toPrint))
		{
			return NULL;
		}
		log << toPrint << std::endl;
		Py_RETURN_NONE;
	}

	PyObject *hash(PyObject * self, PyObject* args)
	{
		const char * hashString;
		if (!PyArg_ParseTuple(args, "s", &hashString))
		{
			return NULL;
		}
		unsigned int value = 0, temp = 0;
		for (size_t i = 0; i<strlen(hashString); i++)
		{
			temp = tolower(hashString[i]) + value;
			value = temp << 10;
			temp += value;
			value = temp >> 6;
			value ^= temp;
		}
		temp = value << 3;
		temp += value;
		unsigned int temp2 = temp >> 11;
		temp = temp2 ^ temp;
		temp2 = temp << 15;
		value = temp2 + temp;
		if (value < 2) value += 2;
		return PyLong_FromLong(value);
	}
};