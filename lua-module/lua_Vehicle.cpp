#include "stdafx.h"

int lua_CreateVehicle(lua_State *L)
{
	lua_pushnumber(L, API::Get().CreateVehicle(API::Get().Hash(lua_tostring(L, 1)), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tointeger(L, 5)));
	return 1;
}

int lua_GetVehicleCoords(lua_State *L)
{

}