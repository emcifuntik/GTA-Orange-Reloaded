#pragma once

#define WIN32_LEAN_AND_MEAN

// Windows Header Files:
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <intrin.h>
#include <string>
#include <locale>
#include <codecvt>
#include <vector>
#include <map>
#include <sstream>
#include <Psapi.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <direct.h>
#include <thread>
#include <iomanip>
#include <TimeAPI.h>

// YAML
#include "yaml-cpp\yaml.h"

// Config
#include "CConfig.h"

// Logging
#include <CConsole.h>
#include <CLog.h>

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

// Scripthook types
#include <../orange-core/core/Types.h>

// Network manager
#include "CNetworkConnection.h"

// Shared stuff from MTA
#include "CMath.h"
#include "CVector3.h"
#include "NetworkTypes.h"

// RPC
#include "CRPCPlugin.h"

// API
#include "API.h"
#include "Plugin.h"

// Network objects
#include "CNetworkPlayer.h"
#include "CNetworkVehicle.h"
#include "CNetworkBlip.h"
#include "CNetworkMarker.h"
#include "CNetwork3DText.h"

//Http Server
#include "CivetServer.h"
#include "CHTTPServer.h"

unsigned long createGUID();