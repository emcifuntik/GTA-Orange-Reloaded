#include "stdafx.h"

int lua_CreateObject(lua_State *L)
{
	if (lua_type(L, 1) == LUA_TSTRING)
		lua_pushnumber(L, API::Get().CreateObject(API::Get().Hash(lua_tostring(L, 1)), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5), lua_tonumber(L, 6), lua_tonumber(L, 7)));
	else
		lua_pushnumber(L, API::Get().CreateObject(lua_tointeger(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5), lua_tonumber(L, 6), lua_tonumber(L, 7)));
	
	return 1;
}

int lua_DeleteObject(lua_State *L)
{
	API::Get().DeleteObject(lua_tointeger(L, 1));
	return 0;
}