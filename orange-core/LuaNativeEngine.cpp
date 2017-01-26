#include "stdafx.h"
#include "Scripting\LuaDefs.h"

struct luameta_t
{
	bool empty = true;
	void* ptr;
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

	for (int i = 0; i < _countof(pointers->data); i++)
	{
		if (pointers->data[i].empty)
		{
			container = &pointers->data[i];
			container->empty = false;

			switch (T)
			{
			case Type::N_BOOL:
			{
				int* value = new int(lua_tointeger(L, 1));
				container->ptr = value;
				break;
			}
			case Type::N_INT:
			case Type::N_INTPOINTER:
			{
				int* value = new int(luaL_checkinteger(L, 1));
				container->ptr = value;
				break;
			}
			case Type::N_INTARRAYPOINTER:
			{
				break;
			}
			case Type::N_FLOAT:
			case Type::N_FLOATPOINTER:
			{
				float* value = new float(luaL_checknumber(L, 1));
				container->ptr = value;
				break;
			}
			case Type::N_STRING:
			{
				const char* val = luaL_checkstring(L, 1);
				int size = strlen(val);
				char* value = new char[size + 1];
				memcpy(value, val, size);
				value[size] = '\0';
				_MY_log << "Str: " << value << ", size: " << size << std::endl;
				container->ptr = value;
				break;
			}
			case Type::N_VECTOR3:
			case Type::N_VECTOR3POINTER:
			{
				/*float* value = new float(luaL_checkinteger(L, 1));
				container->ptr = (uintptr_t)value;*/
				break;
			}
			case Type::N_DWORD:
			case Type::N_DWORDPOINTER:
			{
				int* value = new int(luaL_checkinteger(L, 1));
				container->ptr = value;
				break;
			}
			default:
				_MY_log << "Error getting data" << std::endl;
				break;
			}
			
			/*else if(T == Type::N_STRING)
			{
				const char* val = luaL_checkstring(L, 1);
				int size = strlen(val);
				char* value = new char[size+1];
				memcpy(value, val, size);
				value[size] = '\0';
				log << "Str: " << value << ", size: "<< size << std::endl;
				container->ptr = (uintptr_t)value;
			}
			if (T == Type::N_DWORD || T == Type::N_DWORDPOINTER)
			{
				DWORD* value = new DWORD(luaL_checkinteger(L, 1));
				container->ptr = (uintptr_t)value;
			}*/
			break;
		}
	}
	lua_pushlightuserdata(L, container);
	return 1;
}

int lua_invoke(lua_State *L)
{
	//log << std::hex << lua_tointeger(L, 1) << std::endl;
	//log << *(UINT64*)lua_topointer(L, 1) << std::endl;
	/*log << lua_touserdata(L, 1) << std::endl;
	log << lua_tostring(L, 1) << std::endl;
	log << lua_topointer(L, 1) << std::endl;
	log << std::hex << lua_tonumber(L, 1) << std::endl;
	log << std::hex << static_cast<UINT64>(lua_tonumber(L, 1)) << std::endl << std::endl;*/

	UINT64 hash = *(UINT64*)lua_topointer(L, 1);
	unsigned char retval = lua_tointeger(L, 2);
	int nargs = lua_gettop(L);
	nativeInit(hash);
	int numretn;

	for (int i = 3; i <= nargs; ++i) {
		int type = lua_type(L, i);
		if (type == LUA_TLIGHTUSERDATA)
		{
			luameta_t *ptr = (luameta_t*)lua_touserdata(L, i);
			type = (UINT)(((ULONG)ptr - (ULONG)(&g_pointers)) / sizeof(Entry));

			switch (type)
			{
			case Type::N_INT:
				nativePush(*(int*)ptr->ptr);
				delete (int*)ptr->ptr;
				break;
			case Type::N_INTPOINTER:
				nativePush((int*)ptr->ptr);
				delete (int*)ptr->ptr;
				break;
			case Type::N_DWORD:
				_MY_log << "DWORD: " << *(int*)ptr->ptr << std::endl;
				nativePush(*(int*)ptr->ptr);
				delete (int*)ptr->ptr;
				break;
			case Type::N_DWORDPOINTER:
				nativePush((DWORD*)ptr->ptr);
				delete (DWORD*)ptr->ptr;
				break;
			case Type::N_STRING:
				nativePush((char*)ptr->ptr);
				delete (char*)ptr->ptr;
				break;
			default:
				break;
			}
		}
	}
	switch (retval)
	{
	case Type::N_VOID:
		nativeCall();
		numretn = 0;
		break;
	case Type::N_BOOL:
		lua_pushboolean(L, (*reinterpret_cast<int*>(nativeCall())) != 0);
		numretn = 1;
		break;
	case Type::N_INT:
		lua_pushinteger(L, *reinterpret_cast<int*>(nativeCall()));
		numretn = 1;
		break;
	case Type::N_DWORD:
		lua_pushinteger(L, *reinterpret_cast<int*>(nativeCall()));
		numretn = 1;
		break;
	default:
		_MY_log << "Not impl retn: " << retval;
		break;
	}

	return numretn;
}

static const struct luaL_Reg nativefunclib[] = {
	{ "invoke", lua_invoke },
	{ "bool", lua_getvalue<Type::N_BOOL> },
	{ "int", lua_getvalue<Type::N_INT> },
	{ "intp", lua_getvalue<Type::N_INTPOINTER> },
	{ "intarr", lua_getvalue<Type::N_INTARRAYPOINTER> },
	{ "float", lua_getvalue<Type::N_FLOAT> },
	{ "floatp", lua_getvalue<Type::N_FLOATPOINTER> },
	{ "string", lua_getvalue<Type::N_STRING> },
	{ "vec3", lua_getvalue<Type::N_VECTOR3> },
	{ "vec3p", lua_getvalue<Type::N_VECTOR3POINTER> },
	{ "dword", lua_getvalue<Type::N_DWORD> },
	{ "dwordp", lua_getvalue<Type::N_DWORDPOINTER> },
	{ NULL, NULL }
};

void register_native_funcs(lua_State *L)
{
	lua_newtable(L);
	luaL_setfuncs(L, nativefunclib, 0);
	lua_setglobal(L, "Native");
}