#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <locale>
#include <codecvt>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include <time.h>
#include <thread>
#include <iomanip>

// Windows Header Files:
#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <intrin.h>
#include <Psapi.h>
#include <direct.h>
#include <TimeAPI.h>

#endif

// YAML
#include "yaml-cpp/yaml.h"

// Config
#include "CConfig.h"

#ifdef _WIN32

// Logging
#include <Console/CConsole.h>
#include <CLog.h>

#endif

// RakNet
#include <MessageIdentifiers.h>
#include <RakPeerInterface.h>
#include <RakNetStatistics.h>
#include <RakNetTypes.h>
#include <BitStream.h>
#include <RakSleep.h>
#include <PacketLogger.h>
#include <Gets.h>
#include <WindowsIncludes.h>
#include <GetTime.h>
#include <RPC4Plugin.h>
using namespace RakNet;


#ifndef _WIN32

typedef unsigned long DWORD;
typedef unsigned char BYTE;
typedef unsigned int UINT;
typedef unsigned long ULONG;
#define MAX_PATH 260

#endif

// Scripthook types
#include <types.h>

// Network manager
#include "CNetworkConnection.h"

// Shared stuff from MTA
#include "CMath.h"
#include "CVector3.h"
#include "NetworkTypes.h"

#ifndef _WIN32
#define log std::cout
#define log_debug std::cout
#define log_info std::cout
#define log_error std::cout
#endif

// RPC
#include "CRPCPlugin.h"

// API
#include "API.h"
#include "Plugin.h"
#include "CClientScripting.h"

// Network objects
#include "CNetworkPlayer.h"
#include "CNetworkVehicle.h"
#include "CNetworkBlip.h"
#include "CNetworkMarker.h"
#include "CNetwork3DText.h"
#include "CNetworkObject.h"

//Http Server
#include "CivetServer.h"
#include "CHTTPServer.h"

unsigned long createGUID();
