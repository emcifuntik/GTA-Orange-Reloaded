#pragma once

#define GTA_ORANGE_SERVER

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <locale>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include <time.h>
#include <thread>
#include <iomanip>

// Windows Header Files:
#ifdef _MSC_VER

#define WIN32_LEAN_AND_MEAN

#include <codecvt>
#include <windows.h>
#include <intrin.h>
#include <Psapi.h>
#include <direct.h>
#include <TimeAPI.h>

#else

#include <signal.h>
#include <unistd.h>

#endif

// YAML
#include "yaml-cpp/yaml.h"

// Config
#include "CConfig.h"

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
#include <PacketizedTCP.h>
using namespace RakNet;


#ifndef _WIN32

typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned char BYTE;
typedef unsigned int UINT;
typedef unsigned long ULONG;
typedef int BOOL;
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

// Logging
#include <Console/CConsole.h>
#include <Console/CLogServer.h>

// RPC
#include "CRPCPlugin.h"

// API
#include "API_ext.h"
#include "Plugin.h"
#include "CClientScripting.h"

// Network objects
#include "CNetworkPlayer.h"
#include "CNetworkVehicle.h"
#include "CNetworkBlip.h"
#include "CNetworkMarker.h"
#include "CNetwork3DText.h"
#include "CNetworkObject.h"

unsigned long createGUID();
