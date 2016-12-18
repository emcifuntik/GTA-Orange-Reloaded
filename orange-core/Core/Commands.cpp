#include "stdafx.h"

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}


std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

#include "Natives.h"

int CommandProcessor(std::string command)
{
	std::string fullCmd = command;
	std::vector<std::string> params = split(command, ' ');
	command = params[0];
	params.erase(params.begin());
	if (!command.compare("/quit") || !command.compare("/q"))
	{
		ExitProcess(EXIT_SUCCESS);
		return true;
	}
	if (!command.compare("/save") && CGlobals::Get().isDebug)
	{
		if (!params.size())
		{
			CChat::Get()->AddChatMessage("USAGE: /save [comment]", 0xAAAAAAFF);
			return true;
		}
		std::string comment = fullCmd.substr(command.length() + 1);
		auto playerPed = PLAYER::PLAYER_PED_ID();
		if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, false))
		{
			auto pedVeh = PED::GET_VEHICLE_PED_IS_IN(playerPed, false);
			Vector3 coords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(pedVeh, 0.0, 0.0, 0.0);
			float heading = ENTITY::GET_ENTITY_HEADING(pedVeh);
			std::ofstream saveFile(CGlobals::Get().orangePath + "\\savedcoords.txt", std::ofstream::app);
			saveFile << "{\"vehicle\": { \"coords\": { " << coords.x << ", " << coords.y << ", " << coords.z << "}, \"heading\": " << heading << " }}//" << comment << std::endl;
			saveFile.close();
		}
		else
		{
			Vector3 coords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(playerPed, 0.0, 0.0, 0.0);
			float heading = ENTITY::GET_ENTITY_HEADING(playerPed);
			std::ofstream saveFile(CGlobals::Get().orangePath + "\\savedcoords.txt", std::ofstream::app);
			saveFile << "{\"ped\": { \"coords\": { " << coords.x << ", " << coords.y << ", " << coords.z << "}, \"heading\": " << heading << " }}//" << comment << std::endl;
			saveFile.close();
		}
		CChat::Get()->AddChatMessage("DEBUG: You coordinates saved successfull.", 0xAAFFAAFF);
		return true;
	}
	if (!command.compare("/vehicle") && CGlobals::Get().isDebug)
	{
		if (!params.size())
		{
			CChat::Get()->AddChatMessage("USAGE: /vehicle [modelname]", 0xAAAAAAFF);
			return true;
		}
		Hash c = Utils::Hash(params[0].c_str());
		CScriptInvoker::Get().Push([=]() {
			if (STREAMING::IS_MODEL_IN_CDIMAGE(c) && STREAMING::IS_MODEL_A_VEHICLE(c))
			{
				STREAMING::REQUEST_MODEL(c);
				while (!STREAMING::HAS_MODEL_LOADED(c))
					scriptWait(0);
				Vector3 coords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(PLAYER::PLAYER_PED_ID(), 0.0, 5.0, 0.0);
				Vehicle veh = VEHICLE::CREATE_VEHICLE(c, coords.x, coords.y, coords.z, 0.0, 1, 1);
				VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(veh);

				scriptWait(0);
				STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(c);
				ENTITY::SET_VEHICLE_AS_NO_LONGER_NEEDED(&veh);
			}
		});
		return true;
	}
	if (!command.compare("/snow") && CGlobals::Get().isDebug)
	{
		GAMEPLAY::SET_WEATHER_TYPE_NOW_PERSIST("XMAS");
		GRAPHICS::_SET_FORCE_PED_FOOTSTEPS_TRACKS(true);
		GRAPHICS::_SET_FORCE_VEHICLE_TRAILS(true);
		return true;
	}
	if (!command.compare("/model") && CGlobals::Get().isDebug)
	{
		if (!params.size())
		{
			CChat::Get()->AddChatMessage("USAGE: /model [model id]", 0xAAAAAAFF);
			return true;
		}
		CLocalPlayer::Get()->newModel = GAMEPLAY::GET_HASH_KEY((char*)(models[std::atoi(params[0].c_str())]));
		return true;
	}	
	if (!command.compare("/debug"))
	{
		CGlobals::Get().isDebug ^= 1;
		return true;
	}
	return false;
}