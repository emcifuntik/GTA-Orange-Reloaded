#pragma once
class CPyArgBuilder
{
	std::vector<PyObject*> arguments;
	PyObject* argList = nullptr;
public:
	CPyArgBuilder& operator<<(std::vector<unsigned int> arg)
	{
		size_t listSize = arg.size();
		PyObject *l = PyList_New(listSize);
		for (size_t i = 0; i != listSize; ++i) {
			PyList_SET_ITEM(l, i, PyInt_FromLong((long)arg[i]));
		}
		arguments.push_back(l);
		return *this;
	}
	CPyArgBuilder& operator<<(std::vector<double> arg)
	{
		size_t listSize = arg.size();
		PyObject *l = PyList_New(listSize);
		for (size_t i = 0; i != listSize; ++i) {
			PyList_SET_ITEM(l, i, PyFloat_FromDouble(arg[i]));
		}
		arguments.push_back(l);
		return *this;
	}
	CPyArgBuilder& operator<<(std::vector<float> arg)
	{
		size_t listSize = arg.size();
		PyObject *l = PyList_New(listSize);
		for (size_t i = 0; i != listSize; ++i) {
			PyList_SET_ITEM(l, i, PyFloat_FromDouble((double)arg[i]));
		}
		arguments.push_back(l);
		return *this;
	}
	CPyArgBuilder& operator<<(std::vector<int> arg)
	{
		size_t listSize = arg.size();
		PyObject *l = PyList_New(listSize);
		for (size_t i = 0; i != listSize; ++i) {
			PyList_SET_ITEM(l, i, PyLong_FromLong((long)arg[i]));
		}
		arguments.push_back(l);
		return *this;
	}
	CPyArgBuilder& operator<<(std::vector<long> arg)
	{
		size_t listSize = arg.size();
		PyObject *l = PyList_New(listSize);
		for (size_t i = 0; i != listSize; ++i) {
			PyList_SET_ITEM(l, i, PyLong_FromLong(arg[i]));
		}
		arguments.push_back(l);
		return *this;
	}
	CPyArgBuilder& operator<<(std::vector<std::string> arg)
	{
		size_t listSize = arg.size();
		PyObject *l = PyList_New(listSize);
		for (size_t i = 0; i != listSize; ++i) {
			PyList_SET_ITEM(l, i, PyString_FromString(arg[i].c_str()));
		}
		arguments.push_back(l);
		return *this;
	}
	CPyArgBuilder& operator<<(unsigned int arg)
	{
		arguments.push_back(PyInt_FromLong((long)arg));
		return *this;
	}
	CPyArgBuilder& operator<<(long arg)
	{
		arguments.push_back(PyLong_FromLong(arg));
		return *this;
	}
	CPyArgBuilder& operator<<(int arg)
	{
		arguments.push_back(PyInt_FromLong(arg));
		return *this;
	}
	CPyArgBuilder& operator<<(double arg)
	{
		arguments.push_back(PyFloat_FromDouble(arg));
		return *this;
	}
	CPyArgBuilder& operator<<(const char* arg)
	{
		arguments.push_back(PyString_FromString(arg));
		return *this;
	}
	CPyArgBuilder& operator<<(std::string arg)
	{
		arguments.push_back(PyString_FromString(arg.c_str()));
		return *this;
	}
	PyObject* Finish()
	{
		argList = PyTuple_New(arguments.size());
		int i = 0;
		for each (PyObject* object in arguments)
		{
			PyTuple_SetItem(argList, i, object); //stolen
			i++;
		}
		return argList;
	}
	PyObject* operator()()
	{
		return Finish();
	}

	CPyArgBuilder& operator~()
	{
		arguments.clear();
		if(argList)
			Py_DECREF(argList);
		argList = nullptr;
		return *this;
	}

	CPyArgBuilder();
	~CPyArgBuilder();
};

