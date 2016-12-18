#include "stdafx.h"

int lua_print(lua_State *L)
{
	API::Get().Print(lua_tostring(L, 1));
	return 0;
}

int lua_tick(lua_State *L)
{
	lua_pushvalue(L, 1);

	int ref = luaL_ref(L, LUA_REGISTRYINDEX);
	
	SResource::Get()->SetTick([=]()
	{
		lua_pushvalue(L, 1);

		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

		if (lua_pcall(L, 0, 0, 0) != 0)
		{
			std::string err = luaL_checkstring(L, -1);
			lua_pop(L, 1);

			API::Get().Print(err.c_str());
		}

		lua_pop(L, 1);
	});

	return 0;
}

int lua_HTTPReq(lua_State *L)
{
	lua_pushvalue(L, 1);

	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	SResource::Get()->SetHTTP([=](const char* method, const char* url, const char* query, std::string body)
	{
		lua_pushvalue(L, 1);

		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

		lua_pushstring(L, method);
		lua_pushstring(L, url);
		lua_pushstring(L, query);
		lua_pushstring(L, body.c_str());

		if (lua_pcall(L, 4, 1, 0)) {
			std::string err = luaL_checkstring(L, -1);
			lua_pop(L, 1);

			API::Get().Print(err.c_str());
		}
		if (lua_isnil(L, -1)) {
			lua_pop(L, 2);
			return (char*)NULL;
		}

		char* res = _strdup(lua_tostring(L, -1));
		lua_pop(L, 2);

		return res;
	});

	return 0;
}

int lua_Event(lua_State *L)
{
	lua_pushvalue(L, 1);

	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	SResource::Get()->SetEvent([=](const char* e, std::vector<MValue> *args)
	{
		lua_pushvalue(L, 1);

		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

		lua_pushstring(L, e);

		int count = 1;

		for(int i = 0; i < args->size(); i++)
		{
			count++;
			MValue param = args->at(i);
			switch (param.type)
			{
			case M_BOOL:
				lua_pushboolean(L, param.getBool());
				break;
			case M_INT:
				lua_pushinteger(L, param.getInt());
				break;
			case M_DOUBLE:
				lua_pushnumber(L, param.getDouble());
				break;
			case M_ULONG:
				lua_pushinteger(L, param.getULong());
				break;
			case M_STRING:
				lua_pushstring(L, param.getString());
				break;
			}
		}

		if (lua_pcall(L, count, 0, 0)) {
			std::string err = luaL_checkstring(L, -1);
			lua_pop(L, 1);

			API::Get().Print(err.c_str());
		}

		lua_pop(L, 1);
	});

	return 0;
}

int lua_Create3DText(lua_State *L)
{
	lua_pushinteger(L, API::Get().Create3DText(lua_tostring(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tointeger(L, 5), lua_tointeger(L, 6)));
	return 1;
}