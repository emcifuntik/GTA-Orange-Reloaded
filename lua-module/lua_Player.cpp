#include "stdafx.h"

int lua_GetPlayerCoords(lua_State *L)
{
	CVector3 pos = API::Get().GetPlayerPosition(lua_tointeger(L, 1));

	lua_pushnumber(L, pos.fX);
	lua_pushnumber(L, pos.fY);
	lua_pushnumber(L, pos.fZ);

	return 3;
}

int lua_SetPlayerCoords(lua_State *L)
{
	API::Get().SetPlayerPosition(lua_tointeger(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
	return 0;
}

int lua_SetPlayerModel(lua_State *L)
{
	if (lua_type(L, 2) == LUA_TSTRING)
		lua_pushboolean(L, API::Get().SetPlayerModel(lua_tointeger(L, 1), API::Get().Hash(lua_tostring(L, 2))));
	else
		lua_pushboolean(L, API::Get().SetPlayerModel(lua_tointeger(L, 1), lua_tointeger(L, 2)));
	return 1;
}

int lua_SetPlayerHealth(lua_State *L)
{
	float health = lua_tonumber(L, 2);
	API::Get().SetPlayerHealth(lua_tointeger(L, 1), health == 0 ? 0 : health/2 + 100);
	return 0;
}

int lua_SetPlayerArmour(lua_State *L)
{
	float armour = lua_tonumber(L, 2);
	API::Get().SetPlayerArmour(lua_tointeger(L, 1), armour == 0 ? 0 : armour / 2 + 100);
	return 0;
}

int lua_SetPlayerMoney(lua_State *L)
{
	API::Get().SetPlayerMoney(lua_tointeger(L, 1), lua_tointeger(L, 2));
	return 0;
}

int lua_GetPlayerModel(lua_State *L)
{
	lua_pushinteger(L, API::Get().GetPlayerModel(lua_tointeger(L, 1)));
	return 1;

}
int lua_GetPlayerName(lua_State *L)
{
	std::string name = API::Get().GetPlayerName(lua_tointeger(L, 1));
	lua_pushstring(L, name.c_str());

	return 1;
}

int lua_SetPlayerInfoMsg(lua_State *L)
{
	if(lua_toboolean(L, 2))	API::Get().SetInfoMsg(lua_tointeger(L, 1), lua_tostring(L, 2));
	else API::Get().UnsetInfoMsg(lua_tointeger(L, 1));

	return 0;
}

int lua_SendPlayerNotification(lua_State *L)
{
	API::Get().SendNotification(lua_tointeger(L, 1), lua_tostring(L, 2));

	return 0;
}

int lua_SendPlayerMessage(lua_State *L)
{
	API::Get().SendClientMessage(lua_tointeger(L, 1), lua_tostring(L, 2), 0xFFFFFFFF);

	return 0;
}

int lua_PlayerExists(lua_State *L)
{
	lua_pushboolean(L, true);

	return 1;
}

int lua_GivePlayerWeapon(lua_State *L)
{
	if (lua_type(L, 2) == LUA_TSTRING)
		lua_pushboolean(L, API::Get().GivePlayerWeapon(lua_tointeger(L, 1), API::Get().Hash(lua_tostring(L, 2)), lua_tointeger(L, 3)));
	else
		lua_pushboolean(L, API::Get().GivePlayerWeapon(lua_tointeger(L, 1), lua_tointeger(L, 2), lua_tointeger(L, 3)));

	return 1;
}

int lua_SetPlayerIntoVehicle(lua_State *L)
{
	API::Get().SetPlayerIntoVehicle(lua_tointeger(L, 1), lua_tointeger(L, 2), lua_tointeger(L, 3));
	return 0;
}

int lua_GetPlayerHeading(lua_State *L)
{
	lua_pushnumber(L, API::Get().GetPlayerHeading(lua_tointeger(L, 1)));
	return 1;
}

int lua_SetPlayerHeading(lua_State *L)
{
	API::Get().SetPlayerHeading(lua_tointeger(L, 1), lua_tonumber(L, 2));
	return 0;
}

int lua_RemovePlayerWeapons(lua_State *L)
{
	API::Get().RemovePlayerWeapons(lua_tointeger(L, 1));
	return 0;
}

int lua_GetPlayerGUID(lua_State *L)
{
	lua_pushinteger(L, API::Get().GetPlayerGUID(lua_tointeger(L, 1)));
	return 1;
}

int lua_DisablePlayerHud(lua_State *L)
{
	API::Get().DisablePlayerHud(lua_tointeger(L, 1), lua_toboolean(L, 2));
	return 0;
}

int lua_SetPlayerName(lua_State *L)
{
	API::Get().SetPlayerName(lua_tointeger(L, 1), lua_tostring(L, 2));
	return 0;
}