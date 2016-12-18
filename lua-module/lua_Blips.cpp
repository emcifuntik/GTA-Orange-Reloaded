#include "stdafx.h"

int lua_CreateBlipForAll(lua_State *L)
{
	lua_pushnumber(L, API::Get().CreateBlipForAll(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tointeger(L, 5), lua_tointeger(L, 6)));
	return 1;
}

int lua_CreateBlipForPlayer(lua_State *L)
{
	lua_pushnumber(L, API::Get().CreateBlipForPlayer(lua_tointeger(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5), lua_tointeger(L, 6), lua_tointeger(L, 7)));
	return 1;
}

int lua_DeleteBlip(lua_State *L)
{
	API::Get().DeleteBlip(lua_tointeger(L, 1));
	return 0;
}

int lua_SetBlipColor(lua_State *L)
{
	API::Get().SetBlipColor(lua_tointeger(L, 1), lua_tointeger(L, 2));
	return 0;
}

int lua_SetBlipRoute(lua_State *L)
{
	API::Get().SetBlipRoute(lua_tointeger(L, 1), lua_toboolean(L, 2));
	return 0;
}