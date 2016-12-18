
#pragma once

#ifdef _WINDOWS
#include "targetver.h"

#pragma comment (lib,"lua51.lib")

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#endif

#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <functional>
#include "API.h"

#ifdef _WINDOWS
#include "mysql.h"
#else
#include "orange/mysql/include/mysql.h"
#endif

#include "SResource.h"

#include "lua.hpp"
#include "lua_Main.h"

#ifdef __linux__
char *_strdup(const char *str);
#endif

static void luaL_setfuncs(lua_State *L, const luaL_Reg *l, int nup)
{
	luaL_checkstack(L, nup + 1, "too many upvalues");
	for (; l->name != NULL; l++) {  /* fill the table with given functions */
		int i;
		lua_pushstring(L, l->name);
		for (i = 0; i < nup; i++)  /* copy upvalues to the top */
			lua_pushvalue(L, -(nup + 1));
		lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
		lua_settable(L, -(nup + 3));
	}
	lua_pop(L, nup);  /* remove upvalues */
};

struct Meta
{
	int ref;
	lua_State *m_lua;
};

struct Player
{
	bool exists = false;
	CVector3 pos;
};

extern Player players[256];
