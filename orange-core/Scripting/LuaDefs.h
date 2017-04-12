#pragma once

struct Type {
	enum NType {
		N_VOID,

		N_BOOL,

		N_INT,
		N_INTPOINTER,
		N_INTARRAYPOINTER,

		N_FLOAT,
		N_FLOATPOINTER,

		N_STRING,

		N_VECTOR3,
		N_VECTOR3POINTER,

		N_DWORD,
		N_DWORDPOINTER,

		N_SIZE
	};
};

LUALIB_API void luaL_safeopenlibs(lua_State * L);
void luaL_setfuncs(lua_State *L, const luaL_Reg *l, int nup);

int lua_print(lua_State *L);
int lua_tick(lua_State *L);
int lua_onevent(lua_State *L);
int lua_trigger(lua_State *L);


//Natives
void register_native_funcs(lua_State *L);

//UI
int lua_menu(lua_State *L);
int lua_MenuSetOpened(lua_State *L);
int lua_MenuGetOpened(lua_State *L);
int lua_ChatShown(lua_State *L);

//Buttons
int lua_KeyState(lua_State *L);

//Player
int lua_GetAllPlayers(lua_State *L);
int lua_GetPlayerHandle(lua_State *L);
int lua_GetPlayerName(lua_State *L);
int lua_GetMyGUID(lua_State *L);
int lua_SetHeadDisplay(lua_State *L);
int lua_DisableHead(lua_State *L);

//Vehicle
int lua_GetAllVehicles(lua_State *L);
int lua_GetVehicleHandle(lua_State *L);

//Object
int lua_GetAllObjects(lua_State *L);
int lua_GetObjectHandle(lua_State *L);

//Blip
int lua_GetAllBlips(lua_State *L);
int lua_GetBlipHandle(lua_State *L);



