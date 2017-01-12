#include "stdafx.h"

//just for 3d3 cuz its uniplattform!
#include <sys/types.h>
#include <sys/stat.h>

#ifndef _WIN32
#include <dirent.h>
#include <dlfcn.h>
#endif

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

struct HTTPReq
{
    const char* method;
    const char* url;
    const char* query;
    std::string body;
    const char* result = NULL;
};
std::vector<HTTPReq*> reqquery;
bool queryblocked = false;

const std::string GetRunningExecutableFolder()
{

    char fileName[MAX_PATH];
#ifdef _WIN32
    GetModuleFileNameA(NULL, fileName, MAX_PATH);

    std::string currentPath = fileName;
    return currentPath.substr(0, currentPath.find_last_of("\\"));
#else
    getcwd(fileName, MAX_PATH);
    return fileName;
#endif
}

#ifdef _WIN32
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
        do
        {
            const std::string ppath = pfolder + "\\" + filedat.cFileName;

            std::string modulename = std::string(filedat.cFileName);
            modulename = modulename.substr(0, modulename.size() - 4);

            std::stringstream path;
            path << CConfig::Get()->Path
                 << "\\modules\\" << modulename << "\\bin;" << std::getenv("PATH");

            _putenv_s("PATH", path.str().c_str());

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
        }
        while (FindNextFileA(fileh, &filedat));
        FindClose(fileh);
    }

    for each(auto resource in CConfig::Get()->Resources)
    {
        char path[128];
        sprintf_s(path, 128, "resources\\%s\\resource.yml", resource.c_str());

        std::ifstream fin(path);
        YAML::Parser parser(fin);

        YAML::Node doc;
        if (!parser.GetNextDocument(doc))
        {
            log << "Cant find resource.yml for resource " << resource << std::endl;
        }
        else
        {
            std::string type;
            doc["type"] >> type;

            if (resourceTypes[type])
            {
                resourceTypes[type](resource.c_str());
            }
            else log << "Unknown resource type: " << type << std::endl;
        }
    }
}
#else
bool hasEnding (std::string const &fullString, std::string const &ending)
{
    if (fullString.length() >= ending.length())
    {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    }
    else
    {
        return false;
    }
}

void Plugin::LoadPlugins()
{
    const std::string currentFolder = GetRunningExecutableFolder();
    const std::string pfolder = currentFolder + "/modules";

    DIR *dirp = opendir(pfolder.c_str());
    if(!dirp) log << "No Modules folder found ( " << pfolder << " )" << std::endl;
    else
    {
        dirent *dp = readdir(dirp);
        while (dp != NULL)
        {
            if(dp->d_type == DT_REG && hasEnding(dp->d_name, ".so"))
            {
                std::string modulename = std::string(dp->d_name);
                modulename = modulename.substr(0, modulename.size() - 3);

                std::stringstream path;
                path << pfolder
                     << "/" << modulename << "/bin;" << std::getenv("PATH");

                setenv("PATH", path.str().c_str(), 1);

                const std::string ppath = pfolder + "/" + dp->d_name;

                dlerror();
                void *module = dlopen(ppath.c_str(), RTLD_NOW);
                if(module == NULL)
                    log << "Failed to load \"" << dp->d_name << "\" => 0x" << std::hex << module << std::dec << ". " << dlerror() << std::endl;
                else
                {
                    typedef bool(*Validate_)(API * api);
                    Validate_ validate = (Validate_)dlsym(module, "Validate");
                    if (!validate)
                    {
                        log << "Failed" << std::endl;
                    }
                    else
                    {
                        bool moduleLoaded = validate(API::Get());
                        if (moduleLoaded)
                        {
                            typedef void(*onModuleInit_)();
                            onModuleInit_ OnModuleInit = (onModuleInit_)dlsym(module, "OnModuleInit");
                            OnModuleInit();

                            OnPlayerConnect_ onPlayerConnect = (OnPlayerConnect_)			dlsym(module, "OnPlayerConnect");
                            OnServerCommand_ onServerCommand = (OnServerCommand_)			dlsym(module, "OnServerCommand");
                            OnPlayerDisconnect_ onPlayerDisconnect = (OnPlayerDisconnect_)	dlsym(module, "OnPlayerDisconnect");
                            OnPlayerUpdate_ onPlayerUpdate = (OnPlayerUpdate_)				dlsym(module, "OnPlayerUpdate");
                            OnPlayerCommand_ onPlayerCommand = (OnPlayerCommand_)			dlsym(module, "OnPlayerCommand");
                            OnPlayerText_ onPlayerText = (OnPlayerText_)					dlsym(module, "OnPlayerText");
                            OnTick_ onTick = (OnTick_)										dlsym(module, "OnTick");
                            OnHTTPRequest_ onRequest = (OnHTTPRequest_)						dlsym(module, "OnHTTPRequest");
                            OnEvent_ onEvent = (OnEvent_)									dlsym(module, "OnEvent");

                            if (onPlayerConnect) playerConnects.push_back(onPlayerConnect);
                            if (onServerCommand) serverCommands.push_back(onServerCommand);
                            if (onPlayerDisconnect) playerDisconnects.push_back(onPlayerDisconnect);
                            if (onPlayerUpdate) playerUpdates.push_back(onPlayerUpdate);
                            if (onPlayerCommand) playerCommands.push_back(onPlayerCommand);
                            if (onPlayerText) playerTexts.push_back(onPlayerText);
                            if (onTick) ticks.push_back(onTick);
                            if (onRequest) requests.push_back(onRequest);
                            if (onEvent) eHandlers.push_back(onEvent);

                            OnResourceTypeRegister_ onResourceTypeRegister = (OnResourceTypeRegister_)dlsym(module, "OnResourceTypeRegister");
                            OnResourceLoad_ loadResource = (OnResourceLoad_)dlsym(module, "OnResourceLoad");

                            if (onResourceTypeRegister&&loadResource) resourceTypes[std::string(onResourceTypeRegister())] = loadResource;
                        }
                    }
                }
            }
            dp = readdir(dirp);
        }
        closedir(dirp);
    }
    for (auto resource : CConfig::Get()->Resources)
    {
        char path[MAX_PATH];
        sprintf(path, "resources/%s/resource.yml", resource.c_str());

        std::ifstream fin(path);
        YAML::Parser parser(fin);

        YAML::Node doc;
        if (!parser.GetNextDocument(doc))
        {
            log << "Cant find resource.yml for resource " << resource << std::endl;
        }
        else
        {
            std::string type;
            doc["type"] >> type;

            if (resourceTypes[type])
            {
                resourceTypes[type](resource.c_str());
            }
            else log << "Unknown resource type: " << type << std::endl;
        }
    }
}
#endif

bool Plugin::Tick()
{
    for (HTTPReq *req : reqquery)
    {
        req->result = HTTPRequest(req->method, req->url, req->query, req->body);
    }
    reqquery.clear();
    for (auto func : ticks)
        if (!func())
            return false;
    return true;
}

bool Plugin::PlayerConnect(long playerid)
{
    for (auto func : playerConnects)
        if (!func(playerid))
            return false;
    return true;
}

bool Plugin::ServerCommand(std::string command)
{
    for (auto func : serverCommands)
        if (!func(command))
            return false;
    return true;
}

bool Plugin::PlayerDisconnect(long playerid, int reason)
{
    for (auto func : playerDisconnects)
        if (!func(playerid, reason))
            return false;
    return true;
}

bool Plugin::PlayerUpdate(long playerid)
{
    for (auto func : playerUpdates)
        if (!func(playerid))
            return false;
    return true;
}

bool Plugin::PlayerCommand(long playerid, const char * command)
{
    for (auto func : playerCommands)
        if (!func(playerid, command))
            return false;
    return true;
}

bool Plugin::PlayerText(long playerid, const char * text)
{
    for (auto func : playerTexts)
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
    while (req.result == NULL)
    {
        RakSleep(1);
    }
    return req.result;
}

const char* Plugin::HTTPRequest(const char* method, const char* url, const char* query, std::string body)
{
    HTTPResponse = NULL;
    for (auto func : requests)
    {
        HTTPResponse = func(method, url, query, body);
        if (HTTPResponse != NULL)
        {
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
    for (auto func : eHandlers)
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

