#include "stdafx.h"

int lua_CreateObject(lua_State *L)
{
	lua_pushnumber(L, API::Get().CreateObject(API::Get().Hash(lua_tostring(L, 1)), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5), lua_tonumber(L, 6), lua_tonumber(L, 7)));
	return 1;
}