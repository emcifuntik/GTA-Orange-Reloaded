#pragma once

enum GameVersion {
	GAME_VERSION_STEAM = 1,
	GAME_VERSION_SOCIAL = 2,
	GAME_VERSION_RELOADED = 3
};

class Injector
{
	// ctor & dtor
	Injector();
	~Injector();

	// methods
	int FindProcess(std::wstring procName);
	GameVersion GetGameVersion();
	void WaitUntilGameStarts();
	bool Inject(int processId, std::string dllName);
	void WaitForUnpackFinished(int pid);

	// static fields
	static Injector * instance;

	// fields
	std::vector<std::string> libs;
	bool Injected = false;
public:
	// methods
	void Run(std::wstring folder, std::wstring pePath);
	void RunSteam();
	void InjectAll(bool waitForUnpack = true);
	void PushLibrary(std::string path);


	// static methods
	static Injector& Get() {
		if (!instance)
			instance = new Injector();
		return *instance;
	}
};

