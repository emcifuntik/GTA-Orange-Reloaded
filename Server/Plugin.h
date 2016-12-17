#pragma once

typedef bool(*OnPlayerConnect_)(long playerid);
typedef bool(*OnServerCommand_)(std::string command);
typedef bool(*OnPlayerDisconnect_)(long playerid, int reason);
typedef bool(*OnPlayerUpdate_)(long playerid);
typedef bool(*OnPlayerCommand_)(long playerid, const char * command);
typedef bool(*OnPlayerText_)(long playerid, const char * text);
typedef const char*(*OnResourceTypeRegister_)();
typedef bool(*OnResourceLoad_)(const char* resource);
typedef bool(*OnTick_)();
typedef char*(*OnHTTPRequest_)(const char* method, const char* url, const char* query, std::string body);
typedef void(*OnEvent_)(const char* e, std::vector<MValue>* args);

class Plugin {
	static char* HTTPResponse;

	static std::map<std::string, OnResourceLoad_> resourceTypes;
	static std::vector<OnPlayerConnect_> playerConnects;
	static std::vector<OnServerCommand_> serverCommands;
	static std::vector<OnPlayerDisconnect_> playerDisconnects;
	static std::vector<OnPlayerUpdate_> playerUpdates;
	static std::vector<OnPlayerCommand_> playerCommands;
	static std::vector<OnPlayerText_> playerTexts;
	static std::vector<OnTick_> ticks;
	static std::vector<OnHTTPRequest_> requests;
	static std::vector<OnEvent_> eHandlers;
public:
	static void LoadPlugins();
	static bool Tick();
	static bool PlayerConnect(long playerid);
	static bool ServerCommand(std::string command);
	static bool PlayerDisconnect(long playerid, int reason);
	static bool PlayerUpdate(long playerid);
	static bool PlayerCommand(long playerid, const char * command);
	static bool PlayerText(long playerid, const char * text);
	static const char * OnHTTPRequest(const char * method, const char * url, const char * query, std::string body);
	static const char * HTTPRequest(const char * method, const char * url, const char * query, std::string body);
	static void KeyEvent(RakNet::BitStream * bitStream, RakNet::Packet * packet);

	static void Trigger(const char * e, std::vector<MValue> args);
	static void Trigger(const char * e);
	static void Trigger(const char * e, MValue p0);
	static void Trigger(const char * e, MValue p0, MValue p1);
	static void Trigger(const char * e, MValue p0, MValue p1, MValue p2);
};
