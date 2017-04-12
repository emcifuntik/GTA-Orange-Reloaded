#include "stdafx.h"

int lua_menu(lua_State *L)
{
	lua_pushvalue(L, 1);
	luaL_checktype(L, -1, LUA_TTABLE);

	auto menu = new CMenu();

	lua_getfield(L, -1, "heading");
	if (!lua_isnil(L, -1)) menu->name = _strdup(lua_tostring(L, -1));
	else menu->subname = "Heading";
	lua_pop(L, 1);

	lua_getfield(L, -1, "subheading");
	if (!lua_isnil(L, -1)) menu->subname = _strdup(lua_tostring(L, -1));
	else menu->subname = "here should be subheading";
	lua_pop(L, 1);

	lua_getfield(L, -1, "button");
	if(!lua_isnil(L, -1)) menu->button = lua_tointeger(L, -1);
	else menu->button = -1;
	lua_pop(L, 1);

	lua_getfield(L, -1, "x");
	if (!lua_isnil(L, -1)) menu->pos.fX = lua_tonumber(L, -1);
	else menu->pos.fX = 50;
	lua_pop(L, 1);

	lua_getfield(L, -1, "y");
	if (!lua_isnil(L, -1)) menu->pos.fY = lua_tonumber(L, -1);
	else menu->pos.fY = 50;
	lua_pop(L, 1);

	menu->shown = false;

	lua_getfield(L, -1, "elements");
	lua_rawgeti(L, -1, 1);
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		log << "Menu cant be emprty" << std::endl;
		return 0;
	}
	lua_pop(L, 1);

	for (int i = 1; ; i++) {
		lua_rawgeti(L, -1, i);
		if (lua_isnil(L, -1)) {
			lua_pop(L, 1);
			break;
		}
		luaL_checktype(L, -1, LUA_TTABLE);

		auto child = new CMenuElement;

		child->buffer[0] = '\0';

		lua_rawgeti(L, -1, 1);
		int type = lua_tointeger(L, -1);
		lua_pop(L, 1);

		lua_rawgeti(L, -1, 2);
		const char* capture = lua_tostring(L, -1);
		lua_pop(L, 1);

		child->name = _strdup(capture);
		child->type = type;

		if (type > 0)
		{
			lua_rawgeti(L, -1, 3);

			if (lua_isnil(L, -1)) {
				lua_pop(L, 1);
				child->cb = []() {};
			}
			else {
				int ref = luaL_ref(L, LUA_REGISTRYINDEX);

				switch (type)
				{
				case 1:
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
					break;

				case 2:
					child->cb = [=]()
					{
						lua_pushvalue(L, 1);

						lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

						lua_pushboolean(L, child->state);

						if (lua_pcall(L, 1, 0, 0) != 0)
						{
							std::string err = luaL_checkstring(L, -1);
							lua_pop(L, 1);
							log << err.c_str() << std::endl;
						}

						lua_pop(L, 1);
					};
					break;

				case 3:
					child->cb = [=]()
					{
						lua_pushvalue(L, 1);

						lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

						lua_pushstring(L, child->buffer.c_str());

						if (lua_pcall(L, 1, 0, 0) != 0)
						{
							std::string err = luaL_checkstring(L, -1);
							lua_pop(L, 1);
							log << err.c_str() << std::endl;
						}

						lua_pop(L, 1);
					};
					break;
				}

			}
		}
		//lua_pop(L, 1);

		menu->children.push_back(child);

		lua_pop(L, 1);
	}

	lua_pushinteger(L, CNetworkUI::Get()->AddMenu(menu));
	return 1;
}

int lua_MenuSetOpened(lua_State *L)
{
	CNetworkUI::Get()->menus[lua_tointeger(L, 1)]->shown = lua_toboolean(L, 2);
	return 0;
}

int lua_MenuGetOpened(lua_State *L)
{
	lua_pushboolean(L, CNetworkUI::Get()->menus[lua_tointeger(L, 1)]->shown);
	return 1;
}

int lua_ChatShown(lua_State *L)
{
	lua_pushboolean(L, CChat::Get()->Opened());
	return 1;
}