#include "stdafx.h"

std::vector<Squirrel*> Squirrel::Scripts;
std::vector<FunctionDef> Squirrel::Functions;

void Squirrel::printfunc(HSQUIRRELVM v, const SQChar *s, ...)
{
	va_list vl;
	va_start(vl, s);
	char _str[256];
	scvprintf(_str, s, vl);
	log << _str;
	va_end(vl);
}

void Squirrel::errorfunc(HSQUIRRELVM v, const SQChar *s, ...)
{
	va_list vl;
	va_start(vl, s);
	char _str[256];
	scvprintf(_str, s, vl);
	log << _str;
	va_end(vl);
}

Squirrel::Squirrel(const char *scriptFile)
{
	Scripts.push_back(this);
	vm = sq_open(1024); // creates a VM with initial stack size 1024 

	sqstd_register_bloblib(vm);
	sqstd_register_iolib(vm);
	sqstd_register_systemlib(vm);
	sqstd_register_mathlib(vm);
	sqstd_register_stringlib(vm);

	sqstd_seterrorhandlers(vm); //registers the default error handlers

	sq_setprintfunc(vm, printfunc, errorfunc); //sets the print function

	for each (FunctionDef Function in Functions)
		registerGlobalFunc(Function.CppFunc, Function.SqFunc);

	sq_pushroottable(vm); //push the root table(were the globals of the script will be stored)
	SQRESULT result = sqstd_dofile(vm, _SC(scriptFile), SQFalse, SQTrue);
	_ready = SQ_SUCCEEDED(result);
	if (_ready)
		onScriptLoaded();
}

void Squirrel::onPlayerConnect(int playerId)
{
	SQInteger top = sq_gettop(vm); //saves the stack size before the call
	sq_pushroottable(vm); //pushes the global table
	sq_pushstring(vm, _SC("onPlayerConnect"), -1);
	if (SQ_SUCCEEDED(sq_get(vm, -2))) { //gets the field 'foo' from the global table
		sq_pushroottable(vm); //push the 'this' (in this case is the global table)

		sq_pushinteger(vm, playerId);

		sq_call(vm, 2, SQFalse, SQTrue); //calls the function
	}
	sq_settop(vm, top); //restores the original stack size
}

void Squirrel::PlayerConnect(int playerId)
{
	for each (Squirrel *sq in Scripts)
		sq->onPlayerConnect(playerId);
}

int Squirrel::onPlayerText(int playerId, const char *text, int length)
{
	SQInteger top = sq_gettop(vm); //saves the stack size before the call
	sq_pushroottable(vm); //pushes the global table
	sq_pushstring(vm, _SC("onPlayerText"), -1);
	if (SQ_SUCCEEDED(sq_get(vm, -2))) { //gets the field 'foo' from the global table
		sq_pushroottable(vm); //push the 'this' (in this case is the global table)

		sq_pushinteger(vm, playerId);
		sq_pushstring(vm, text, length);
		
		sq_call(vm, 2, SQFalse, SQTrue); //calls the function 
	}
	sq_settop(vm, top); //restores the original stack size
	return 0;
}

int Squirrel::PlayerText(int playerId, const char *text, int length)
{
	for each (Squirrel *sq in Scripts)
	{
		if (sq->onPlayerText(playerId, text, length))
			return 1;
	}
	return 0;
}

int Squirrel::onPlayerCommand(int playerId, const char *text, int length)
{
	SQInteger top = sq_gettop(vm); //saves the stack size before the call
	sq_pushroottable(vm); //pushes the global table
	sq_pushstring(vm, _SC("onPlayerCommand"), -1);
	if (SQ_SUCCEEDED(sq_get(vm, -2))) { //gets the field 'foo' from the global table
		sq_pushroottable(vm); //push the 'this' (in this case is the global table)

		sq_pushinteger(vm, playerId);
		sq_pushstring(vm, text, length);

		sq_call(vm, 2, SQFalse, SQTrue); //calls the function 
	}
	sq_settop(vm, top); //restores the original stack size
	return 0;
}

int Squirrel::PlayerCommand(int playerId, const char *text, int length)
{
	for each (Squirrel *sq in Scripts)
	{
		if (sq->onPlayerCommand(playerId, text, length))
			return 1;
	}
	return 0;
}

SQBool Squirrel::onPlayerUpdate(int playerId)
{
	SQBool result;
	SQInteger top = sq_gettop(vm); //saves the stack size before the call
	sq_pushroottable(vm); //pushes the global table
	sq_pushstring(vm, _SC("onPlayerUpdate"), -1);
	if (SQ_SUCCEEDED(sq_get(vm, -2))) { //gets the field 'foo' from the global table
		sq_pushroottable(vm); //push the 'this' (in this case is the global table)

		sq_pushinteger(vm, playerId);

		result = sq_call(vm, 2, SQTrue, SQTrue); //calls the function 
	}
	sq_settop(vm, top); //restores the original stack size
	return (SQBool)result;
}

SQBool Squirrel::PlayerUpdate(int playerId)
{
	SQBool result = SQTrue;
	for each (Squirrel *sq in Scripts)
	{
		if (!sq->onPlayerUpdate(playerId))
			result = SQFalse;
	}
	return result;
}

void Squirrel::onPlayerDisconnect(int playerId, int reason)
{
	SQInteger top = sq_gettop(vm); //saves the stack size before the call
	sq_pushroottable(vm); //pushes the global table
	sq_pushstring(vm, _SC("onPlayerDisonnect"), -1);
	if (SQ_SUCCEEDED(sq_get(vm, -2))) { //gets the field 'foo' from the global table
		sq_pushroottable(vm); //push the 'this' (in this case is the global table)
		sq_pushinteger(vm, playerId);
		sq_pushinteger(vm, reason);
		sq_call(vm, 3, SQFalse, SQTrue); //calls the function 
	}
	sq_settop(vm, top); //restores the original stack size
}
void Squirrel::PlayerDisconnect(int playerId, int reason)
{
	for each (Squirrel *sq in Scripts)
		sq->onPlayerDisconnect(playerId, reason);
}
void Squirrel::onScriptLoaded()
{
	SQInteger top = sq_gettop(vm); //saves the stack size before the call
	sq_pushroottable(vm); //pushes the global table
	sq_pushstring(vm, _SC("onScriptLoaded"), -1);
	if (SQ_SUCCEEDED(sq_get(vm, -2))) { //gets the field 'foo' from the global table
		sq_pushroottable(vm); //push the 'this' (in this case is the global table)
		sq_call(vm, 1, SQFalse, SQTrue); //calls the function 
	}
	sq_settop(vm, top); //restores the original stack size
}
void Squirrel::onScriptUnloaded()
{
	SQInteger top = sq_gettop(vm); //saves the stack size before the call
	sq_pushroottable(vm); //pushes the global table
	sq_pushstring(vm, _SC("onScriptUnloaded"), -1);
	if (SQ_SUCCEEDED(sq_get(vm, -2))) { //gets the field 'foo' from the global table
		sq_pushroottable(vm); //push the 'this' (in this case is the global table)
		sq_call(vm, 1, SQFalse, SQTrue); //calls the function 
	}
	sq_settop(vm, top); //restores the original stack size
}

SQInteger registerEvent(HSQUIRRELVM v)
{

}

SQInteger Squirrel::registerGlobalFunc(SQFUNCTION f, const char *fname)
{
	sq_pushroottable(vm);
	sq_pushstring(vm, fname, -1);
	sq_newclosure(vm, f, 0); //create a new function
	sq_newslot(vm, -3, SQFalse);
	sq_pop(vm, 1); //pops the root table
	return 1;
}

void Squirrel::AddFunction(SQFUNCTION f, const char * fname)
{
	Functions.push_back({ f, fname });
}

void Squirrel::Close()
{
	onScriptUnloaded();
	sq_pop(vm, 1); //pops the root table
	sq_close(vm);
}

void Squirrel::Destroy()
{
	while (Scripts.size())
	{
		Scripts[0]->Close();
		delete Scripts[0];
		Scripts.erase(Scripts.begin(), Scripts.begin() + 1);
	}
}
