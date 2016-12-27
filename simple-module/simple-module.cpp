
#include "stdafx.h"

API * API::instance = nullptr;

extern "C"
{
	__declspec(dllexport) bool Validate(API * api)
	{
		API::Set(api);
		return true;
	}

	__declspec(dllexport) void OnModuleInit()
	{
		API::Get().Print("Simple module has been successfully loaded!");

		API::Get().CreateVehicle(0x6322B39A, 46.34f, -688.664f, 43.6748f, 133.994f);
		API::Get().CreateVehicle(0x6322B39A, 43.0207f, -695.123f, 43.662f, 159.652f);
		API::Get().CreateVehicle(0x6322B39A, 40.2774f, -702.291f, 43.6512f, 158.812f);
		API::Get().CreateVehicle(0x6322B39A, 38.2197f, -707.548f, 43.6416f, 158.856f);

		API::Get().Print("All cars were loaded!");
	}

	__declspec(dllexport) bool OnPlayerConnect(long playerid)
	{
		std::stringstream message;
		message << "Player " << API::Get().GetPlayerName(playerid) << " joined the server!";
		API::Get().BroadcastClientMessage(message.str().c_str(), 0xFFFFFFFF);
		API::Get().SetPlayerPosition(playerid, 21.2369f, -711.042f, 45.973f);
		return true;
	}

	__declspec(dllexport) bool OnServerCommand(std::string command)
	{

		return true;
	}

	__declspec(dllexport) bool OnPlayerDisconnect(long playerid, int reason)
	{
		std::stringstream message;
		message << "Player  " << API::Get().GetPlayerName(playerid) << " left the server(" << ((reason == 1) ? "Disconnected" : "Timeout") << ")!";
		API::Get().BroadcastClientMessage(message.str().c_str(), 0xFFFFFFFF);
		return true;
	}

	__declspec(dllexport) bool OnPlayerUpdate(long playerid)
	{

		return true;
	}

	__declspec(dllexport) bool OnPlayerCommand(long playerid, const char * command)
	{

		return true;
	}

	__declspec(dllexport) bool OnPlayerText(long playerid, const char * text)
	{

		return true;
	}
}

