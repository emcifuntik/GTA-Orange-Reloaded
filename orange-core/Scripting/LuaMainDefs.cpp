#include "stdafx.h"
#include "LuaDefs.h"

static const luaL_Reg lj_libs[] = {
	{ "",			luaopen_base },
	{ LUA_LOADLIBNAME,	luaopen_package },
	{ LUA_TABLIBNAME,	luaopen_table },
	{ LUA_STRLIBNAME,	luaopen_string },
	{ LUA_MATHLIBNAME,	luaopen_math },
	{ LUA_DBLIBNAME,	luaopen_debug },
	{ LUA_BITLIBNAME,	luaopen_bit },
	{ NULL,		NULL }
};

int lua_print(lua_State *L)
{
	int nargs = lua_gettop(L);
	my_ostream& ss = my_ostream::_log();
	for (int i = 1; i <= nargs; ++i) {
		ss << lua_tostring(L, i) << "\t";
	}
	ss << std::endl;

	return 0;
}

int lua_tick(lua_State *L)
{
	lua_pushvalue(L, 1);

	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	CScriptEngine::Get()->SetTick([=]()
	{
		lua_pushvalue(L, 1);

		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

		if (lua_pcall(L, 0, 0, 0) != 0)
		{
			std::string err = luaL_checkstring(L, -1);
			lua_pop(L, 1);
			log << err.c_str() << std::endl;
		}

		lua_pop(L, 1);
	});

	return 0;
}

int lua_menu(lua_State *L)
{

	lua_pushvalue(L, 5);
	luaL_checktype(L, -1, LUA_TTABLE);

	lua_rawgeti(L, -1, 1);
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		log << "Menu cant be emprty" << std::endl;
		return 0;
	}
	lua_pop(L, 1);

	auto menu = new CMenu();

	menu->name = _strdup(lua_tostring(L, 1));
	menu->button = lua_tointeger(L, 2);
	menu->pos.fX = lua_tonumber(L, 3);
	menu->pos.fY = lua_tonumber(L, 4);

	menu->shown = false;

	for (int i = 1; ; i++) {
		lua_rawgeti(L, -1, i);
		if (lua_isnil(L, -1)) {
			lua_pop(L, 1);
			break;
		}
		luaL_checktype(L, -1, LUA_TTABLE);

		auto child = new CMenuElement;

		lua_rawgeti(L, -1, 1);
		int type = lua_tointeger(L, -1);
		lua_pop(L, 1);

		lua_rawgeti(L, -1, 2);
		const char* capture = lua_tostring(L, -1);
		lua_pop(L, 1);

		child->name = _strdup(capture);
		child->type = type;
	
		if (type == 1)
		{
			lua_rawgeti(L, -1, 3);

			if (lua_isnil(L, -1)) {
				lua_pop(L, 1);
				child->cb = []() {};
			}
			else {
				int ref = luaL_ref(L, LUA_REGISTRYINDEX);
				child->cb = [=]()
				{
					lua_pushvalue(L, 1);

					lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

					if (lua_pcall(L, 0, 0, 0) != 0)
					{
						std::string err = luaL_checkstring(L, -1);
						lua_pop(L, 1);
						log << err.c_str() << std::endl;
					}

					lua_pop(L, 1);
				};
			}
		}
		//lua_pop(L, 1);

		menu->children.push_back(child);

		log << "Table entry(" << type << "): " << capture << std::endl;

		lua_pop(L, 1);
	}

	CNetworkUI::Get()->AddMenu(menu);
	return 0;
}

int lua_trigger(lua_State *L)
{
	BitStream bsOut;
	int nargs = lua_gettop(L);

	RakString name(lua_tostring(L, 1));
	bsOut.Write(name);
	bsOut.Write(nargs-1);

	//log << "Event: " << name << " " << nargs - 1 << std::endl;

	for (int i = 2; i <= nargs; ++i) {
		switch (lua_type(L, i))
		{
		case LUA_TBOOLEAN:
		{
			bool val = lua_toboolean(L, i);
			bsOut.Write(0);
			bsOut.Write(val);
			break;
		}
		case LUA_TNUMBER:
		{
			bsOut.Write(1);
			bsOut.Write((double)lua_tonumber(L, i));
			break;
		}
		case LUA_TSTRING:
		{
			RakString str(lua_tostring(L, i));
			bsOut.Write(2);
			bsOut.Write(str);
			break;
		}
		default:
			log << "You can only pass bools, numbers and strings" << std::endl;
			break;
		}
	}
	CRPCPlugin::Get()->rpc.Signal("ServerEvent", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
	return 0;
}

void luaL_setfuncs(lua_State *L, const luaL_Reg *l, int nup)
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

LUALIB_API void luaL_safeopenlibs(lua_State *L)
{
	const luaL_Reg *lib;
	for (lib = lj_libs; lib->func; lib++) {
		lua_pushcfunction(L, lib->func);
		lua_pushstring(L, lib->name);
		lua_call(L, 1, 0);
	}
}