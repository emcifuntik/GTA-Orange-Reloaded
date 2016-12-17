#include <Python.h>
class Python
{
	Python();
	static Python* singleInstance;
	
	std::vector<PyObject *> pModules;
public:
	static Python* Get();
	long pCallFunc(char * fName, PyObject* args);
	bool Connect(const char * script_name);
	void DefineMethods();
	~Python()
	{
		for each (PyObject *module in pModules)
		{
			Py_DECREF(module);
		}
		Py_Finalize();
	}
};