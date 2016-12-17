#include "stdafx.h"

CPyArgBuilder::CPyArgBuilder()
{
}


CPyArgBuilder::~CPyArgBuilder()
{
	if (argList)
		Py_DECREF(argList);
}
