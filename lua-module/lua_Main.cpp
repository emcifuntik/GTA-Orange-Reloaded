#include "stdafx.h"

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
	API::Get().Print(ss.str().c_str());
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

		char* res = strdup(lua_tostring(L, -1));
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

		std::stringstream ss;

		for(int i = 0; i < args->size(); i++)
		{
			count++;
			MValue param = args->at(i);
			ss << " ";
			switch (param.type)
			{
			case M_BOOL:
				ss << param.getBool();
				lua_pushboolean(L, param.getBool());
				break;
			case M_INT:
				ss << param.getInt();
				lua_pushinteger(L, param.getInt());
				break;
			case M_DOUBLE:
				ss << param.getDouble();
				lua_pushnumber(L, param.getDouble());
				break;
			case M_ULONG:
				ss << param.getULong();
				lua_pushinteger(L, param.getULong());
				break;
			case M_STRING:
				ss << param.getString();
				lua_pushstring(L, param.getString());
				break;
			}
		}

		//API::Get().Print(ss.str().c_str());

		if (lua_pcall(L, count, 0, 0)) {
			std::string err = luaL_checkstring(L, -1);
			lua_pop(L, 1);

			API::Get().Print(err.c_str());
		}

		lua_pop(L, 1);
	});

	return 0;
}

int lua_Command(lua_State *L)
{
	lua_pushvalue(L, 1);

	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	SResource::Get()->SetCommandProcessor([=](long pid, const char* cmd)
	{
		lua_pushvalue(L, 1);

		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

		lua_pushinteger(L, pid);
		lua_pushstring(L, cmd);

		if (lua_pcall(L, 2, 1, 0)) {
			std::string err = luaL_checkstring(L, -1);
			lua_pop(L, 1);

			API::Get().Print(err.c_str());
		}
		if (lua_isnil(L, -1)) {
			lua_pop(L, 2);
			return true;
		}

		bool res = lua_toboolean(L, -1);
		lua_pop(L, 2);

		return res;
	});

	return 0;
}

int lua_Text(lua_State *L)
{
	lua_pushvalue(L, 1);

	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	SResource::Get()->SetTextProcessor([=](long pid, const char* text)
	{
		lua_pushvalue(L, 1);

		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

		lua_pushinteger(L, pid);
		lua_pushstring(L, text);

		if (lua_pcall(L, 2, 1, 0)) {
			std::string err = luaL_checkstring(L, -1);
			lua_pop(L, 1);

			API::Get().Print(err.c_str());
		}
		if (lua_isnil(L, -1)) {
			lua_pop(L, 2);
			return true;
		}

		bool res = lua_toboolean(L, -1);
		lua_pop(L, 2);

		return res;
	});

	return 0;
}

int lua_trigger(lua_State *L)
{
	int nargs = lua_gettop(L);
	const char* name = lua_tostring(L, 1);
	long player = lua_tointeger(L, 2);

	std::vector<MValue> args;

	for (int i = 3; i <= nargs; ++i) {
		switch (lua_type(L, i))
		{
		case LUA_TBOOLEAN:
		{
			bool val = lua_toboolean(L, i);
			args.push_back(val);
			break;
		}
		case LUA_TNUMBER:
		{
			args.push_back(lua_tonumber(L, i));
			break;
		}
		case LUA_TSTRING:
		{
			args.push_back(lua_tostring(L, i));
			break;
		}
		case LUA_TTABLE:
		{
			MValue arr = MValue::CreateMArray();
			lua_pushvalue(L, i);
			lua_pushnil(L);

			while (lua_next(L, -2))
			{
				// stack now contains: -1 => value; -2 => key; -3 => table
				lua_pushvalue(L, -2);
				// stack now contains: -1 => key; -2 => value; -3 => key; -4 => table

				int keytype = lua_type(L, -1);

				if (keytype != LUA_TNUMBER && keytype != LUA_TSTRING)
				{
					API::Get().Print("You can only use numbers and strings as keys");
					return 0;
				}
				
				switch (lua_type(L, -2))
				{
				case LUA_TBOOLEAN:
				{
					bool val = lua_toboolean(L, -2);
					if (keytype == LUA_TNUMBER) arr.push((int)lua_tointeger(L, -1), val);
					else arr.push(lua_tostring(L, -1), val);
					break;
				}
				case LUA_TNUMBER:
				{
					double val = lua_tonumber(L, -2);
					if (keytype == LUA_TNUMBER) arr.push((int)lua_tointeger(L, -1), val);
					else arr.push(lua_tostring(L, -1), val);
					break;
				}
				case LUA_TSTRING:
				{
					const char* val = lua_tostring(L, -2);
					if (keytype == LUA_TNUMBER) arr.push((int)lua_tointeger(L, -1), val);
					else arr.push(lua_tostring(L, -1), val);
					break;
				}
				case LUA_TTABLE:
				{
					API::Get().Print("Nested tables are unsupported yet");
					break;
				}
				default:
					break;
				}

				lua_pop(L, 2);
				// stack now contains: -1 => key; -2 => table
			}
			lua_pop(L, 1);

			args.push_back(arr);
			break;
		}
		default:
			API::Get().Print("You can only pass bools, numbers and strings");
			break;
		}
	}
	API::Get().ClientEvent(name, args, player);

	return 0;
}

int lua_Create3DText(lua_State *L)
{
	lua_pushinteger(L, API::Get().Create3DText(lua_tostring(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tointeger(L, 5), lua_tointeger(L, 6), lua_tonumber(L, 7)));
	return 1;
}

int lua_Delete3DText(lua_State *L)
{
	API::Get().Delete3DText(lua_tointeger(L, 1));
	return 0;
}

int lua_Set3DTextText(lua_State *L)
{
	API::Get().Set3DTextContent(lua_tointeger(L, 1), lua_tostring(L, 2));
	return 0;
}

int lua_Attach3DTextToVeh(lua_State *L)
{
	API::Get().Attach3DTextToVehicle(lua_tointeger(L, 1), lua_tointeger(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5));
	return 0;
}

int lua_Attach3DTextToPlayer(lua_State *L)
{
	API::Get().Attach3DTextToPlayer(lua_tointeger(L, 1), lua_tointeger(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5));
	return 0;
}

int lua_LoadClientScript(lua_State *L)
{
	SResource::Get()->AddClientScript(lua_tostring(L, 1));
	return 0;
}

int lua_Delete3DTextToPlayer(lua_State *L)
{
	API::Get().Delete3DText(lua_tointeger(L, 1));
	return 0;
}

int lua_Broadcast(lua_State *L)
{
	API::Get().BroadcastClientMessage(lua_tostring(L, 1), 0xFFFFFFFF);
	return 0;
}

int lua_Hash(lua_State *L)
{
	lua_pushinteger(L, API::Get().Hash(lua_tostring(L, 1)));
	return 1;
}
