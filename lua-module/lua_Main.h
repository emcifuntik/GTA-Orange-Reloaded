#pragma once

//Main
int lua_print(lua_State *L);
int lua_tick(lua_State *L);
int lua_HTTPReq(lua_State * L);
int lua_Event(lua_State *L);
int lua_Command(lua_State *L);
int lua_LoadClientScript(lua_State *L);

//Player
int lua_GetPlayerCoords(lua_State *L);
int lua_GetPlayerModel(lua_State *L);
int lua_GetPlayerName(lua_State *L);
int lua_GivePlayerWeapon(lua_State *L);
int lua_SetPlayerCoords(lua_State *L);
int lua_SendPlayerNotification(lua_State *L);
int lua_SetPlayerInfoMsg(lua_State *L);
int lua_SendPlayerMessage(lua_State *L);
int lua_PlayerExists(lua_State *L);
int lua_SetPlayerIntoVehicle(lua_State *L);

//Vehicle
int lua_CreateVehicle(lua_State *L);

//Object
int lua_CreateObject(lua_State *L);

//Blips
int lua_CreateBlipForAll(lua_State *L);
int lua_CreateBlipForPlayer(lua_State *L);
int lua_DeleteBlip(lua_State *L);
int lua_SetBlipColor(lua_State *L);
int lua_SetBlipRoute(lua_State *L);

//Markers
int lua_CreateMarkerForAll(lua_State *L);
int lua_CreateMarkerForPlayer(lua_State *L);
int lua_DeleteMarker(lua_State *L);

//MySQL
int luaopen_luasql_mysql(lua_State *L);

//3DText
int lua_Create3DText(lua_State *L);
int lua_Set3DTextText(lua_State *L);
int lua_Attach3DTextToVeh(lua_State *L);
int lua_Attach3DTextToPlayer(lua_State *L);
