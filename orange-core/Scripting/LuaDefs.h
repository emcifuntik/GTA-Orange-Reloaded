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

int lua_print(lua_State *L);
int lua_tick(lua_State *L);
void luaL_setfuncs(lua_State *L, const luaL_Reg *l, int nup);

//Natives
void register_native_funcs(lua_State *L);

//UI
int lua_menu(lua_State *L);
