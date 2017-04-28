#pragma once

#define WIN32_LEAN_AND_MEAN

#ifdef _WINDOWS
#include "targetver.h"
#include <windows.h>
#include <string>
#else
#include <cstring>
#include <string.h>
#endif

#include <sstream>
#include <vector>
#include <map>
#include <iostream>
#include <Python.h>

#include "API.h"

#include "python_Main.h"