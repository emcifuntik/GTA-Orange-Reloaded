#include "stdafx.h"
#include "Scripting\LuaDefs.h"

struct luameta_t
{
	bool empty = true;
	uintptr_t ptr;
};

struct Entry
{
	luameta_t data[32];
};

Entry g_pointers[Type::N_SIZE];
BYTE g_metas[Type::N_SIZE];

template<Type::NType T>
int lua_getmeta(lua_State *L)
{
	lua_pushlightuserdata(L, &g_metas[T]);
	return 1;
}

template<Type::NType T>
int lua_getvalue(lua_State *L)
{
	Entry *pointers = &g_pointers[T];
	luameta_t *container = nullptr;
	std::stringstream ss;

	for (int i = 0; i < _countof(pointers->data); i++)
	{
		if (pointers->data[i].empty)
		{
			container = &pointers->data[i];
			container->empty = false;

			if (T == Type::N_INT || T == Type::N_INTPOINTER)
			{
				int* value = new int(luaL_checkinteger(L, 1));
				container->ptr = (uintptr_t)value;
			}

			break;
		}
	}
	ss << "Get: " << container;
	log << ss.str().c_str() << std::endl;
	lua_pushlightuserdata(L, container);
	return 1;
}

int lua_invoke(lua_State *L)
{
	log << std::hex << lua_tointeger(L, 1) << std::endl;
	log << lua_touserdata(L, 1) << std::endl;
	log << lua_tostring(L, 1) << std::endl;
	log << lua_topointer(L, 1) << std::endl;
	log << std::hex << lua_tonumber(L, 1) << std::endl;
	log << std::hex << static_cast<UINT64>(lua_tonumber(L, 1)) << std::endl << std::endl;

	return 0;
	/*UINT64 hash = lua_tointeger(L, 1);
	log << "[L] Init" << std::endl;
	unsigned char retval = lua_tointeger(L, 2);
	int nargs = lua_gettop(L);


	if(nargs > 2) nativeInit(0x21F191D9AFF98B5E);
	else nativeInit(hash);

	log << "Hash: " << std::hex  << hash << " " << 0x21F191D9AFF98B5E << std::endl;
	std::stringstream ss;

	for (int i = 3; i <= nargs; ++i) {
		int type = lua_type(L, i);
		if (type == LUA_TLIGHTUSERDATA)
		{
			luameta_t *ptr = (luameta_t*)lua_touserdata(L, i);
			type = (UINT)(((ULONG)ptr - (ULONG)(&g_pointers)) / sizeof(Entry));

			switch (type)
			{
			case Type::N_INT:
			{
				int val = *(int*)ptr->ptr;
				log << "Pushing: " << val << std::endl;
				nativePush(0);
				log << "[L] Push" << std::endl;
				delete (int*)ptr->ptr;
				break;
			}
			case Type::N_INTPOINTER:
				log << "intptr: " << *(int*)(ptr->ptr);
				nativePush((int*)ptr->ptr);
				delete (int*)ptr->ptr;
				break;
			default:
				break;
			}
		}
	}
	//log << ss.str().c_str() << std::endl;
	log << "[L] Call" << std::endl;
	int result = *reinterpret_cast<int*>(nativeCall());

	nativeInit(0x21F191D9AFF98B5E);
	nativePush(0);
	int result2 = *reinterpret_cast<int*>(nativeCall());

	lua_pushinteger(L, result);
	log << std::hex << result << " " << result2 << " " << PLAYER::PLAYER_PED_ID() << std::endl;

	return 1;*/
}

static const struct luaL_Reg nativefunclib[] = {
	{ "__invoke", lua_invoke },
	{ "_i", lua_getvalue<Type::N_INT> },
	{ "_ip", lua_getvalue<Type::N_INTPOINTER> },
	{ "_ia", lua_getvalue<Type::N_INTARRAYPOINTER> },
	{ "_f", lua_getvalue<Type::N_FLOAT> },
	{ "_fp", lua_getvalue<Type::N_FLOATPOINTER> },
	{ NULL, NULL }
};

void register_native_funcs(lua_State *L)
{
	lua_newtable(L);
	luaL_setfuncs(L, nativefunclib, 0);
	lua_setglobal(L, "Native");
}