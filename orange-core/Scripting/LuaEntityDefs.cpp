#include "stdafx.h"

int lua_GetAllPlayers(lua_State *L)
{
	auto players = CNetworkPlayer::All();
	int i = 1;

	lua_newtable(L);

	lua_pushinteger(L, RakNetGUID::ToUint32(CNetworkConnection::Get()->client->GetMyGUID()));
	lua_rawseti(L, -2, i++);

	for (auto pl : players) {
		if (!pl) continue;
		//log << pl->GetGUID().ToString() << pl->GetName() << std::endl;
		lua_pushinteger(L, RakNetGUID::ToUint32(pl->GetGUID()));
		lua_rawseti(L, -2, i++);
	}

	return 1;
}

int lua_GetPlayerHandle(lua_State *L)
{
	auto pl = CNetworkPlayer::GetByGUID32(lua_tointeger(L, 1));
	//log << RakNetGUID(lua_tointeger(L, 1)).ToString() << std::endl;
	if (pl)
	{
		//log << pl->GetName() << std::endl;
		lua_pushinteger(L, pl->GetHandle());
		return 1;
	}
	else if (lua_tointeger(L, 1) == RakNetGUID::ToUint32(CNetworkConnection::Get()->client->GetMyGUID()))
	{
		lua_pushinteger(L, CLocalPlayer::Get()->GetHandle());
		return 1;
	}
	return 0;
}

int lua_GetPlayerName(lua_State *L)
{
	//log << lua_tointeger(L, 1) << " " << RakNetGUID::ToUint32(CNetworkConnection::Get()->client->GetMyGUID()) << std::endl;
	auto pl = CNetworkPlayer::GetByGUID32(lua_tointeger(L, 1));
	if (pl)
	{
		lua_pushstring(L, pl->GetName().c_str());
		return 1;
	}
	else if (lua_tointeger(L, 1) == RakNetGUID::ToUint32(CNetworkConnection::Get()->client->GetMyGUID()))
	{
		lua_pushstring(L, CGlobals::Get().name.c_str());
		return 1;
	}
	return 0;
}

int lua_SetHeadDisplay(lua_State *L)
{
	lua_pushvalue(L, 1);

	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	CScriptEngine::Get()->SetCustomHead([=](unsigned long guid, bool clear, float distance, CVector3 pos)
	{
		lua_pushvalue(L, 1);

		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

		lua_pushinteger(L, guid);
		lua_pushboolean(L, clear);
		lua_pushnumber(L, distance);
		lua_pushnumber(L, pos.fX);
		lua_pushnumber(L, pos.fY);

		if (lua_pcall(L, 5, 0, 0) != 0)
		{
			std::string err = luaL_checkstring(L, -1);
			lua_pop(L, 1);
			log << err.c_str() << std::endl;
		}

		lua_pop(L, 1);
	});

	return 0;
}

int lua_DisableHead(lua_State *L)
{
	CNetworkPlayer::enableHead = !lua_toboolean(L, 1);
	return 0;
}

int lua_GetAllVehicles(lua_State *L)
{
	lua_newtable(L);

	auto vehicles = CNetworkVehicle::All();
	int i = 1;

	for (auto veh : vehicles) {
		if (!veh) continue;
		lua_pushinteger(L, RakNetGUID::ToUint32(veh->m_GUID));
		lua_rawseti(L, -2, i++);
	}

	return 1;
}

int lua_GetVehicleHandle(lua_State *L)
{
	auto veh = CNetworkVehicle::GetByGUID(RakNetGUID(lua_tointeger(L, 1)));
	if (veh)
	{
		lua_pushinteger(L, veh->GetHandle());
		return 1;
	}
	return 0;
}

int lua_GetAllObjects(lua_State *L)
{
	lua_newtable(L);

	auto objects = CNetworkVehicle::All();
	int i = 1;

	for (auto obj : objects) {
		if (!obj) continue;
		lua_pushinteger(L, RakNetGUID::ToUint32(obj->m_GUID));
		lua_rawseti(L, -2, i++);
	}

	return 1;
}

int lua_GetObjectHandle(lua_State *L)
{
	auto obj = CNetworkVehicle::GetByGUID(RakNetGUID(lua_tointeger(L, 1)));
	if (obj)
	{
		lua_pushinteger(L, obj->GetHandle());
		return 1;
	}
	return 0;
}

int lua_GetAllBlips(lua_State *L)
{
	lua_newtable(L);

	auto blips = CNetworkVehicle::All();
	int i = 1;

	for (auto blip : blips) {
		if (!blip) continue;
		lua_pushinteger(L, RakNetGUID::ToUint32(blip->m_GUID));
		lua_rawseti(L, -2, i++);
	}

	return 1;
}

int lua_GetBlipHandle(lua_State *L)
{
	auto blip = CNetworkVehicle::GetByGUID(RakNetGUID(lua_tointeger(L, 1)));
	if (blip)
	{
		lua_pushinteger(L, blip->GetHandle());
		return 1;
	}
	return 0;
}

int lua_GetMyGUID(lua_State *L)
{
	lua_pushinteger(L, RakNetGUID::ToUint32(CNetworkConnection::Get()->client->GetMyGUID()));
	return 1;
}