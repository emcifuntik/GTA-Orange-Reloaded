#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
//#include <stdlib.h>
#include "API.h"

#define NODE_WANT_INTERNALS true
#include "env.h"
#include "env-inl.h"
//#include "util.h"
//#include "util-inl.h"
#include "v8.h"
#include "uv.h"
#include "node.h"
#include "libplatform/libplatform.h"

using namespace std;
using namespace v8;

#include "uv_callback/uv_callback.h"
#include "NodeModule.h"