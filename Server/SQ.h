#pragma once
#ifdef SQUNICODE 

#define scvprintf vswprintf
#else 

#define scvprintf vsprintf
#endif 

typedef struct funcDef {
	SQFUNCTION CppFunc;
	const char *SqFunc;
} FunctionDef;

class Squirrel {
private:
	HSQUIRRELVM vm;
	bool _ready = false;
	void onScriptLoaded();
	void onScriptUnloaded();
	static std::vector<Squirrel*> Scripts;
	static std::vector<FunctionDef> Functions;
public:
	Squirrel(const char * scriptFile);

	static void printfunc(HSQUIRRELVM v, const SQChar *s, ...);
	static void errorfunc(HSQUIRRELVM v, const SQChar *s, ...);
	static void AddFunction(SQFUNCTION f, const char *fname);

	SQInteger registerGlobalFunc(SQFUNCTION f, const char *fname);

	void onPlayerConnect(int playerId);
	static void PlayerConnect(int playerId);
	int onPlayerText(int playerId, const char * text, int length);
	static int PlayerText(int playerId, const char * text, int length);
	int onPlayerCommand(int playerId, const char * text, int length);
	static int PlayerCommand(int playerId, const char * text, int length);
	SQBool onPlayerUpdate(int playerId);
	static SQBool PlayerUpdate(int playerId);
	void onPlayerDisconnect(int playerId, int reason);
	static void PlayerDisconnect(int playerId, int reason);


	bool IsReady() { return _ready; }

	void Close();
	static void Destroy();
};