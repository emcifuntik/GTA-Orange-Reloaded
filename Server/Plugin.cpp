#include "stdafx.h"

//just for 3d3 cuz its uniplattform!
#include <sys/types.h>
#include <sys/stat.h>

struct stat info;
char* Plugin::HTTPResponse = NULL;
std::map<std::string, OnResourceLoad_> Plugin::resourceTypes;
std::vector<OnPlayerConnect_> Plugin::playerConnects;
std::vector<OnServerCommand_> Plugin::serverCommands;
std::vector<OnPlayerDisconnect_> Plugin::playerDisconnects;
std::vector<OnPlayerUpdate_> Plugin::playerUpdates;
std::vector<OnPlayerCommand_> Plugin::playerCommands;
std::vector<OnPlayerText_> Plugin::playerTexts;
std::vector<OnTick_> Plugin::ticks;
std::vector<OnHTTPRequest_> Plugin::requests;
std::vector<OnEvent_> Plugin::eHandlers;

struct HTTPReq {
	const char* method;
	const char* url;
	const char* query;
	std::string body;
	const char* result = NULL;
};
std::vector<HTTPReq*> reqquery;
bool queryblocked = false;

const std::string GetRunningExecutableFolder() {

	char fileName[MAX_PATH];
	GetModuleFileNameA(NULL, fileName, MAX_PATH);

	std::string currentPath = fileName;
	return currentPath.substr(0, currentPath.find_last_of("\\"));
}

void Plugin::LoadPlugins()
{
	const std::string currentFolder = GetRunningExecutableFolder();
	const std::string pfolder = currentFolder + "\\modules";
	const std::string searchfolder = pfolder + "\\*.dll";

	WIN32_FIND_DATAA filedat;
	HANDLE fileh = FindFirstFileA(searchfolder.c_str(), &filedat);

	if (stat(pfolder.c_str(), &info) != 0)
	{
		std::stringstream ss;
		ss << "No Modules folder found ( " << pfolder << " )" << std::endl;
		log_info << ss.str()  << std::endl;
	}
	if (fileh != INVALID_HANDLE_VALUE)
	{
		do {
			const std::string ppath = pfolder + "\\" + filedat.cFileName;
			HMODULE module = LoadLibraryA(ppath.c_str());
			if (!module)
			{
				std::stringstream ss;
				ss << "Failed to load \"" << filedat.cFileName << "\" => 0x" << std::hex << module << std::dec << std::endl;
				log_error << ss.str() << std::endl;
			}
			else
			{
				typedef bool(*Validate_)(API * api);
				Validate_ validate = (Validate_)GetProcAddress(module, "Validate");
				if (!validate)
				{
					std::stringstream ss;
					ss << "Failed to load \"" << filedat.cFileName << "\" => 0x" << std::hex << module << std::dec << ". Didn\'t pass validation" << std::endl;
					log_error << ss.str() << std::endl;
				}
				else
				{
					bool moduleLoaded = validate(API::Get());
					if (moduleLoaded)
					{
						typedef void(*onModuleInit_)();
						onModuleInit_ OnModuleInit = (onModuleInit_)GetProcAddress(module, "OnModuleInit");
						OnModuleInit();

						OnPlayerConnect_ onPlayerConnect = (OnPlayerConnect_)			GetProcAddress(module, "OnPlayerConnect");
						OnServerCommand_ onServerCommand = (OnServerCommand_)			GetProcAddress(module, "OnServerCommand");
						OnPlayerDisconnect_ onPlayerDisconnect = (OnPlayerDisconnect_)	GetProcAddress(module, "OnPlayerDisconnect");
						OnPlayerUpdate_ onPlayerUpdate = (OnPlayerUpdate_)				GetProcAddress(module, "OnPlayerUpdate");
						OnPlayerCommand_ onPlayerCommand = (OnPlayerCommand_)			GetProcAddress(module, "OnPlayerCommand");
						OnPlayerText_ onPlayerText = (OnPlayerText_)					GetProcAddress(module, "OnPlayerText");
						OnTick_ onTick = (OnTick_)										GetProcAddress(module, "OnTick");
						OnHTTPRequest_ onRequest = (OnHTTPRequest_)						GetProcAddress(module, "OnHTTPRequest");
						OnEvent_ onEvent = (OnEvent_)									GetProcAddress(module, "OnEvent");

						if (onPlayerConnect) playerConnects.push_back(onPlayerConnect);
						if (onServerCommand) serverCommands.push_back(onServerCommand);
						if (onPlayerDisconnect) playerDisconnects.push_back(onPlayerDisconnect);
						if (onPlayerUpdate) playerUpdates.push_back(onPlayerUpdate);
						if (onPlayerCommand) playerCommands.push_back(onPlayerCommand);
						if (onPlayerText) playerTexts.push_back(onPlayerText);
						if (onTick) ticks.push_back(onTick);
						if (onRequest) requests.push_back(onRequest);
						if (onEvent) eHandlers.push_back(onEvent);

						OnResourceTypeRegister_ onResourceTypeRegister = (OnResourceTypeRegister_)GetProcAddress(module, "OnResourceTypeRegister");
						OnResourceLoad_ loadResource = (OnResourceLoad_)GetProcAddress(module, "OnResourceLoad");

						if (onResourceTypeRegister&&loadResource) resourceTypes[std::string(onResourceTypeRegister())] = loadResource;
					}
				}
			}
		} while (FindNextFileA(fileh, &filedat));
		FindClose(fileh);
		
		for each(auto resource in CConfig::Get()->Resources)
		{
			char path[128];
			sprintf_s(path, 128, "resources\\%s\\resource.yml", resource.c_str());

			std::ifstream fin(path);
			YAML::Parser parser(fin);

			YAML::Node doc;
			if (!parser.GetNextDocument(doc)) {
				log << "Cant find resource.yml for resource " << resource << std::endl;
			}
			else {
				std::string type;
				doc["type"] >> type;

				if (resourceTypes[type]) {
					resourceTypes[type](resource.c_str());
				}
				else log << "Unknown resource type: " << type << std::endl;
			}
		}
	}
}

bool Plugin::Tick()
{
	for each (HTTPReq *req in reqquery) {
		req->result = HTTPRequest(req->method, req->url, req->query, req->body);
	}
	reqquery.clear();
	for each (auto func in ticks)
		if (!func())
			return false;
	return true;
}

bool Plugin::PlayerConnect(long playerid)
{
	for each (auto func in playerConnects)
		if (!func(playerid))
			return false;
	return true;
}

bool Plugin::ServerCommand(std::string command)
{
	for each (auto func in serverCommands)
		if (!func(command))
			return false;
	return true;
}

bool Plugin::PlayerDisconnect(long playerid, int reason)
{
	for each (auto func in playerDisconnects)
		if (!func(playerid, reason))
			return false;
	return true;
}

bool Plugin::PlayerUpdate(long playerid)
{
	for each (auto func in playerUpdates)
		if (!func(playerid))
			return false;
	return true;
}

bool Plugin::PlayerCommand(long playerid, const char * command)
{
	for each (auto func in playerCommands)
		if (!func(playerid, command))
			return false;
	return true;
}

bool Plugin::PlayerText(long playerid, const char * text)
{
	for each (auto func in playerTexts)
		if (!func(playerid, text))
			return false;
	return true;
}

const char* Plugin::OnHTTPRequest(const char* method, const char* url, const char* query, std::string body)
{
	HTTPReq req;
	req.method = method;
	req.url = url;
	req.query = query;
	req.body = body;
	while (queryblocked) RakSleep(1);
	reqquery.push_back(&req);
	while (req.result == NULL) {
		RakSleep(1);
	}
	return req.result;
}

const char* Plugin::HTTPRequest(const char* method, const char* url, const char* query, std::string body)
{
	HTTPResponse = NULL;
	for each (auto func in requests)
	{
		HTTPResponse = func(method, url, query, body);
		if (HTTPResponse != NULL) {
			return HTTPResponse;
		}
	}
	return NULL;
}

void Plugin::KeyEvent(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	DWORD keycode;
	bitStream->Read(keycode);
	Plugin::Trigger("keyPress", (ULONG)CNetworkPlayer::GetByGUID(packet->guid)->GetID(), keycode);
}

void Plugin::Trigger(const char* e, std::vector<MValue> args)
{
	for each (auto func in eHandlers)
	{
		func(e, &args);
	}
}

void Plugin::Trigger(const char* e)
{
	std::vector<MValue> params;

	Trigger(e, params);
}

void Plugin::Trigger(const char* e, MValue p0)
{
	std::vector<MValue> params;

	params.push_back(p0);

	Trigger(e, params);
}

void Plugin::Trigger(const char* e, MValue p0, MValue p1)
{
	std::vector<MValue> params;

	params.push_back(p0);
	params.push_back(p1);

	Trigger(e, params);
}

void Plugin::Trigger(const char* e, MValue p0, MValue p1, MValue p2)
{
	std::vector<MValue> params;

	params.push_back(p0);
	params.push_back(p1);
	params.push_back(p2);

	Trigger(e, params);
}

