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
	std::vector<std::string> params = split(command, ' ');
	command = params[0];
	params.erase(params.begin());
	if (!command.compare("/quit") || !command.compare("/q") || !command.compare("/exit"))
	{
		ExitProcess(EXIT_SUCCESS);
		return true;
	}
	if (!command.compare("/vehicle"))
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
	if (!command.compare("/snow"))
	{
		GAMEPLAY::SET_WEATHER_TYPE_NOW_PERSIST("XMAS");
		GRAPHICS::_SET_FORCE_PED_FOOTSTEPS_TRACKS(true);
		GRAPHICS::_SET_FORCE_VEHICLE_TRAILS(true);
		return true;
	}
	if (!command.compare("/model"))
	{
		if (!params.size())
		{
			CChat::Get()->AddChatMessage("USAGE: /model [model id]", 0xAAAAAAFF);
			return true;
		}
		CLocalPlayer::Get()->newModel = GAMEPLAY::GET_HASH_KEY((char*)(models[std::atoi(params[0].c_str())]));
		return true;
	}

	if (!command.compare("/freemode"))
	{
		CChat::Get()->AddChatMessage("Changed!", 0xAAAAAAFF);
		CLocalPlayer::Get()->newModel = GAMEPLAY::GET_HASH_KEY("mp_m_freemode_01");
		return true;
	}
	
#if _DEBUG
	if (!command.compare("/model"))
	{
		if (!params.size())
		{
			CChat::Get()->AddChatMessage("USAGE: /model [model id]", 0xAAAAAAFF);
			return true;
		}
		CLocalPlayer::Get()->newModel = GAMEPLAY::GET_HASH_KEY((char*)(models[std::atoi(params[0].c_str())]));
		return true;
	}
	if (!command.compare("/anim"))
	{
		if (!params.size())
		{
			CChat::Get()->AddChatMessage("USAGE: /anim [ped id]", 0xAAAAAAFF);
			return true;
		}
		int handler = std::atoi(params[0].c_str());
		int setToPed = std::atoi(params[1].c_str());
		auto pool = &ReplayInterfaces::Get()->ReplayInterfacePed->pool;
		int pedID = -1;
		for (int i = 0; i < pool->Capacity(); ++i)
		{
			if (pool->GetHandle(i) == handler)
			{
				pedID = i;
				break;
			}
		}
		if (pedID == -1)
		{
			CChat::Get()->AddChatMessage("ERROR: Ped with this id not found", 0xFFAAAAFF);
			return true;
		}
		CPed *ped = (*pool)[pedID];
		GTA::CTask *parent = nullptr;
		for (GTA::CTask *task = CWorld::Get()->CPedPtr->TasksPtr->PrimaryTasks->GetTask(); task; task = task->SubTask)
		{
			if (!task->IsSerializable())
				continue;
			auto ptr = task->Serialize();
			if (ptr)
			{
				//log_debug << "Size: " << ptr->Size() << std::endl;
				auto nextTask = (GTA::CTask*) ptr->GetTask();
				nextTask->Deserialize(ptr);
				if (!parent)
				{
					log_debug << "0x" << std::hex << nextTask << std::endl;
					if (setToPed)
						ped->TasksPtr->PrimaryTasks->AssignTask(nextTask, GTA::TASK_PRIORITY_HIGH);
					parent = nextTask;
				}
				else
				{
					GTA::CTask *lastChild;
					for (lastChild = parent; lastChild->SubTask; lastChild = lastChild->SubTask);
					
				}
				rage::sysMemAllocator::Get()->free(ptr, rage::HEAP_TASK_CLONE);
			}
			break;
		}
		return true;
	}
	if (!command.compare("/debug"))
	{
		CGlobals::Get().isDebug ^= 1;
		return true;
	}
#endif
	return false;
}