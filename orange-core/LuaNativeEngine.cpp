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
				int* value = new int(lua_toboolean(L, 1));
				delete (int*)container->ptr;
				container->ptr = value;
				break;
			}
			case Type::N_INT:
			case Type::N_INTPOINTER:
			{
				int* value = new int(lua_tointeger(L, 1));
				delete (int*)container->ptr;
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
				float* value = new float(lua_tonumber(L, 1));
				delete (float*)container->ptr;
				container->ptr = value;
				break;
			}
			case Type::N_STRING:
			{
				if (lua_isnumber(L, 1) || lua_isboolean(L, 1))
				{
					container->ptr = (void*)lua_tointeger(L, 1);
				}
				else
				{
					const char* val = luaL_checkstring(L, 1);
					int size = strlen(val);
					char* value = new char[size + 1];
					memcpy(value, val, size);
					value[size] = '\0';
					delete (char*)container->ptr;
					container->ptr = value;
				}
				break;
			}
			case Type::N_VECTOR3:
			case Type::N_VECTOR3POINTER:
			{
				break;
			}
			case Type::N_DWORD:
			case Type::N_DWORDPOINTER:
			{
				DWORD* value = new DWORD(luaL_checkinteger(L, 1));
				delete (DWORD*)container->ptr;
				container->ptr = value;
				break;
			}
			default:
				log << "Error getting data" << std::endl;
				break;
			}

			break;
		}
	}
	lua_pushlightuserdata(L, container);
	return 1;
}

void LuaInvoke(uint64_t hash, ScriptManagerContext *ctx)
{
	auto fn = ScriptEngine::GetNativeHandler(hash);
	if (fn != 0) {
		__try {
			fn(ctx);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			log_error << "[LUA] Error in nativeCall. 0x" << hash << std::endl;
		}
	}
}

template <typename T>
void LuaPush(ScriptManagerContext *ctx, T val)
{
	UINT64 val64 = 0;
	if (sizeof(T) > sizeof(UINT64))
	{
		throw "error, value size > 64 bit";
	}
	*reinterpret_cast<T *>(&val64) = val; // &val + sizeof(dw) - sizeof(val)
	ctx->Push(val64);
}

int lua_invoke(lua_State *L)
{
	UINT64 hash = *(UINT64*)lua_topointer(L, 1);
	unsigned char retval = lua_tointeger(L, 2);

	int nargs = lua_gettop(L);
	int numretn = 0;

	int numpretn = 0;
	luameta_t* preturns[8];

	ScriptManagerContext ctx;
	
	//log << "Called 0x" << hash << std::endl;
	//std::stringstream ss;
	//ss << "Params: ";

	for (int i = 3; i <= nargs; ++i) {
		int type = lua_type(L, i);
		if (type == LUA_TLIGHTUSERDATA)
		{
			luameta_t *ptr = (luameta_t*)lua_touserdata(L, i);
			type = (UINT)(((ULONG)ptr - (ULONG)(&g_pointers)) / sizeof(Entry));

			ptr->empty = true;

			switch (type)
			{
			case Type::N_BOOL:
				//ss << *(int*)ptr->ptr;
				LuaPush(&ctx, *(int*)ptr->ptr);
				break;
			case Type::N_INT:
				//ss << *(int*)ptr->ptr;
				LuaPush(&ctx, *(int*)ptr->ptr);
				break;
			case Type::N_INTPOINTER:
				//ss << *(int*)ptr->ptr;
				LuaPush(&ctx, (int*)ptr->ptr);
				preturns[numpretn++] = ptr;
				break;
			case Type::N_FLOAT:
				//ss << *(float*)ptr->ptr;
				LuaPush(&ctx, *(float*)ptr->ptr);
				break;
			case Type::N_FLOATPOINTER:
				//ss << *(float*)ptr->ptr;
				LuaPush(&ctx, (float*)ptr->ptr);
				preturns[numpretn++] = ptr;
				break;
			case Type::N_DWORD:
				//ss << *(int*)ptr->ptr;
				LuaPush(&ctx, *(int*)ptr->ptr);
				break;
			case Type::N_DWORDPOINTER:
				//ss << *(DWORD*)ptr->ptr;
				LuaPush(&ctx, (DWORD*)ptr->ptr);
				preturns[numpretn++] = ptr;
				break;
			case Type::N_STRING:
				//ss << (char*)ptr->ptr;
				LuaPush(&ctx, _strdup((char*)ptr->ptr));
				break;
			default:
				break;
			}
			//ss << ", ";
		}
	}

	//log << ctx.GetArgumentCount()  << ss.str() << std::endl;

	LuaInvoke(hash, &ctx);

	switch (retval)
	{
	case Type::N_VOID:
		break;
	case Type::N_BOOL:
		lua_pushboolean(L, (*reinterpret_cast<int*>(ctx.GetResultPointer())) != 0);
		numretn += 1;
		break;
	case Type::N_INT:
		lua_pushinteger(L, *reinterpret_cast<int*>(ctx.GetResultPointer()));
		numretn += 1;
		break;
	case Type::N_FLOAT:
		lua_pushnumber(L, *reinterpret_cast<float*>(ctx.GetResultPointer()));
		numretn += 1;
		break;
	case Type::N_VECTOR3:
	{
		Vector3 pos = *reinterpret_cast<Vector3*>(ctx.GetResultPointer());
		lua_pushnumber(L, pos.x);
		lua_pushnumber(L, pos.y);
		lua_pushnumber(L, pos.z);
		numretn += 3;
		break;
	}
	case Type::N_DWORD:
		lua_pushinteger(L, *reinterpret_cast<int*>(ctx.GetResultPointer()));
		numretn += 1;
		break;
	default:
		log << "Not impl retn: " << (int)retval;
		break;
	}

	numretn += numpretn;

	for (int i = 0; i < numpretn; i++)
	{
		luameta_t* ptr = preturns[i];
		unsigned int type = (UINT)(((ULONG)ptr - (ULONG)(&g_pointers)) / sizeof(Entry));

		switch (type)
		{
		case Type::N_INTPOINTER:
			//log << *(int*)(ptr->ptr) << std::endl;
			lua_pushinteger(L, *(int*)(ptr->ptr));
			break;
		case Type::N_FLOATPOINTER:
			//log << *(float*)(ptr->ptr) << std::endl;
			lua_pushnumber(L, *(float*)(ptr->ptr));
			break;
		case Type::N_DWORDPOINTER:
			//log << *(DWORD*)(ptr->ptr) << std::endl;
			lua_pushinteger(L, *(DWORD*)(ptr->ptr));
			break;
		default:
			numretn--;
			log << "not supported pointer " << type << std::endl;
		}
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