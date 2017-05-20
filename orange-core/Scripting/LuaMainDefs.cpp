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
	std::stringstream ss;
	int nargs = lua_gettop(L);
	for (int i = 1; i <= nargs; ++i) {
		switch (lua_type(L, i))
		{
		case LUA_TNIL:
			ss << "nil";
			break;
		case LUA_TBOOLEAN:
			ss << (lua_toboolean(L, i) ? "true" : "false");
			break;
		case LUA_TTABLE:
			ss << "[table]";
			break;
		case LUA_TTHREAD:
			ss << "[thread]";
			break;
		case LUA_TUSERDATA:
			ss << "[userdata]";
			break;
		case LUA_TLIGHTUSERDATA:
			ss << "[lightuserdata]";
			break;
		case LUA_TFUNCTION:
			ss << "[function]";
			break;
		case LUA_TSTRING:
		case LUA_TNUMBER:
			ss << lua_tostring(L, i);
			break;
		default:
			ss << "[cant print dat]";
			break;
		}
		ss << "\t";
	}
	log << ss.str() << std::endl;;
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

int lua_onevent(lua_State *L)
{
	lua_pushvalue(L, 1);

	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	CScriptEngine::Get()->SetEvent([=](RakNet::BitStream *bsIn)
	{
		lua_pushvalue(L, 1);

		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

		RakString e;
		bsIn->Read(e);
		lua_pushstring(L, e.C_String());

		int count;
		bsIn->Read(count);

		for (int i = 0; i < count; i++)
		{
			char type;
			bsIn->Read(type);
			switch (type)
			{
			case 0:
			{
				bool val;
				bsIn->Read(val);
				lua_pushboolean(L, val);
				break;
			}
			case 1:
			{
				double val;
				bsIn->Read(val);
				lua_pushnumber(L, val);
				break;
			}
			case 2:
			{
				RakString val;
				bsIn->Read(val);
				lua_pushstring(L, val.C_String());
				break;
			}
			case 3:
			{
				unsigned int size;
				bsIn->Read(size);
				lua_newtable(L);

				log << "TABLE:" << std::endl;

				for (int i = 0; i < size; i++)
				{
					char atype;
					bsIn->Read(atype);

					if (atype & 16)
					{
						int key;
						bsIn->Read(key);
						log << key << " => ";
						lua_pushinteger(L, key);
					}
					else
					{
						std::string key;
						bsIn->Read(key);
						log << key << " => ";
						lua_pushstring(L, key.c_str());
					}

					switch (atype & 15)
					{
					case 0:
					{
						bool val;
						bsIn->Read(val);
						std::cout << val;
						lua_pushboolean(L, val);
						break;
					}
					case 1:
					{
						double val;
						bsIn->Read(val);
						std::cout << val;
						lua_pushnumber(L, val);
						break;
					}
					case 2:
					{
						std::string val;
						bsIn->Read(val);
						std::cout << val;
						lua_pushstring(L, val.c_str());
						break;
					}
					}
					std::cout << std::endl;
					lua_settable(L, -3);
				}
				break;
			}
			}
		}

		if (lua_pcall(L, count+1, 0, 0)) {
			log << luaL_checkstring(L, -1) << std::endl;
			lua_pop(L, 1);
		}

		lua_pop(L, 1);
	});

	return 0;
}

int lua_KeyState(lua_State *L)
{
	lua_pushboolean(L, (GetKeyState(lua_tointeger(L, 1)) & 0x8000) != 0);
	return 1;
}

int lua_GetTime(lua_State *L)
{
	lua_pushinteger(L, timeGetTime());
	return 1;
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