#include "stdafx.h"

CNetworkConnection *CNetworkConnection::singleInstance = nullptr;

CNetworkConnection::CNetworkConnection()
{
	client = RakNet::RakPeerInterface::GetInstance();
	tcpclient = RakNet::PacketizedTCP::GetInstance();
}

CNetworkConnection::~CNetworkConnection()
{
	
}

CNetworkConnection * CNetworkConnection::Get()
{
	if (!singleInstance)
		singleInstance = new CNetworkConnection();
	return singleInstance;
}

bool CNetworkConnection::Connect(std::string host, unsigned short port)
{
	if (!host.empty() && port)
	{
		RakNet::SocketDescriptor socketDescriptor(0, 0);

		socketDescriptor.socketFamily = AF_INET;

		client->Startup(8, &socketDescriptor, 1);
		client->SetOccasionalPing(true);
		connection = client->Connect(host.c_str(), port, 0, 0);
		RakAssert(connection == RakNet::CONNECTION_ATTEMPT_STARTED);

		if (connection == RakNet::CONNECTION_ATTEMPT_STARTED)
		{
			tcpclient->Start(0, 0);
			tcpclient->Connect(host.c_str(), port);

			SystemAddress tcpaddr = tcpclient->HasCompletedConnectionAttempt();
			//while (tcpaddr == UNASSIGNED_SYSTEM_ADDRESS) RakSleep(0);

			BitStream con;

			con.Write((unsigned char)1);
			con.Write(client->GetMyGUID());

			tcpclient->Send(reinterpret_cast<char*>(con.GetData()), con.GetNumberOfBytesUsed(), tcpaddr, false);

			bConnected = true;
			CRPCPlugin::Get();
			return true;
		}

		return false;
	}
	return false;
}

void CNetworkConnection::Disconnect()
{
	bConnected = false;
	tcpclient->Stop();
	client->Shutdown(300);
	CChat::Get()->Clear();
	CChat::Get()->AddChatMessage("Disconnected");
	CNetworkPlayer::Clear();
}

void CNetworkConnection::Tick()
{
	for (packet = client->Receive(); packet; client->DeallocatePacket(packet), packet = client->Receive()) {
		unsigned char packetID = packet->data[0];
		RakNet::BitStream bsIn(packet->data, packet->length, false);
		RakNet::BitStream bsOut;
		bsIn.IgnoreBytes(sizeof(unsigned char));

		switch (packetID) {
			case ID_CONNECTION_REQUEST_ACCEPTED:
			{
				CLocalPlayer::Get()->FreezePosition(false);
				CLocalPlayer::Get()->SetVisible(true);
				CLocalPlayer::Get()->SetMoney(0);

				cEstablished++;
				if (cEstablished > 1)
				{
					RakString playerName(CConfig::Get()->sNickName.c_str());
					bsOut.Write((unsigned char)ID_CONNECT_TO_SERVER);
					bsOut.Write(playerName);

					client->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
				}
				break;
			}
			case ID_CONNECTION_ATTEMPT_FAILED:
			{
				CLocalPlayer::Get()->SetMoney(0);
				CChat::Get()->AddChatMessage("Not connected");
				break;
			}
			case ID_NO_FREE_INCOMING_CONNECTIONS:
			{
				CLocalPlayer::Get()->SetMoney(0);
				CChat::Get()->AddChatMessage("Server is full!");
				break;
			}
			case ID_DISCONNECTION_NOTIFICATION:
			{
				CLocalPlayer::Get()->SetMoney(0);

				CChat::Get()->AddChatMessage("You were kicked!");

				break;
			}
			case ID_CONNECTION_LOST:
			{
				CLocalPlayer::Get()->SetMoney(0);
				CChat::Get()->AddChatMessage("Connection Lost!");
				break;
			}
			case ID_CONNECTION_BANNED:
			{
				CLocalPlayer::Get()->SetMoney(0);
				CChat::Get()->AddChatMessage("You are banned!");
				break;
			}
			case ID_CONNECT_TO_SERVER:
			{
				bEstablished = true;
				break;
			}
			case ID_SEND_PLAYER_DATA:
			{
				OnFootSyncData data;
				RakNet::RakNetGUID playerGUID;
				RakNet::RakString rsName;
				bsIn.Read(playerGUID);
				bsIn.Read(rsName);
				bsIn.Read(data);
				CNetworkPlayer::hFutureModel = data.hModel;
				CNetworkPlayer *remotePlayer = CNetworkPlayer::GetByGUID(playerGUID);

				if(rsName.GetLength())
					remotePlayer->SetName(std::string(rsName.C_String()));

				remotePlayer->UpdateLastTickTime();
				remotePlayer->SetOnFootData(data, 100);
				if (data.bShooting)
					remotePlayer->Interpolate();
				break;
			}
			case ID_SEND_VEHICLE_DATA:
			{
				VehicleData data;
				RakNet::RakNetGUID vehGUID;
				RakNet::RakString rsName;
				bsIn.Read(data);

				if (data.GUID != UNASSIGNED_RAKNET_GUID)
				{
					CNetworkVehicle *remoteVeh = CNetworkVehicle::GetByGUID(data.GUID);
					if (remoteVeh)
					{
						remoteVeh->UpdateLastTickTime();
						remoteVeh->SetVehicleData(data, 100 + (int)(data.vecMoveSpeed.Length()*1.6));
					}
				}

				break;
			}
			case ID_SEND_TASKS:
			{
				RakNet::RakNetGUID playerGUID;
				bsIn.Read(playerGUID);
				CNetworkPlayer * player = CNetworkPlayer::GetByGUID(playerGUID);
				std::vector<TaskPair> ClonedTasks;
				int parentTaskID = -1;
				if (player)
				{
					int tasks;
					bsIn.Read(tasks);
					for (int i = 0; i < tasks; i++)
					{
						unsigned short taskID;
						bsIn.Read(taskID);
						log_debug << "Recieved " << VTasks::Get()->GetTaskName(taskID) << std::endl;

						unsigned int size;
						bsIn.Read(size);

						int bytesSize = (size % 8) ? (size / 8 + 1) : (size / 8);
						unsigned char* taskInfo = new unsigned char[bytesSize];
						bsIn.ReadBits(taskInfo, size);
						rageBuffer data;
						typedef void(*InitBuffer)(rageBuffer*);
						((InitBuffer)CMemory((uintptr_t)GetModuleHandle(NULL) + 0x11E7920)())(&data);
						typedef void(*InitReadBuffer)(rageBuffer*, unsigned char*, int, int);
						((InitReadBuffer)CMemory((uintptr_t)GetModuleHandle(NULL) + 0x11EBCA8)())(&data, taskInfo, size, 0);
						typedef CSerialisedFSMTaskInfo*(*CreateTaskInfoByID)(unsigned int);
						CSerialisedFSMTaskInfo* serTask = ((CreateTaskInfoByID)CMemory((uintptr_t)GetModuleHandle(NULL) + 0x0658904)())(taskID);

						serTask->Read(&data);
						ClonedTasks.push_back({ serTask, taskID });
						delete[size] taskInfo;
						if (parentTaskID == -1)
							parentTaskID = taskID;
					}
					if (parentTaskID != -1)
					{
						GTA::CTask *parentTask = nullptr;
						GTA::CTask *cursorTask = nullptr;
						for each (auto cloned in ClonedTasks)
						{
							if (!parentTask)
							{
								parentTask = (GTA::CTask*)cloned.task->GetTask();
								if (parentTask)
								{
									parentTask->Deserialize(cloned.task);
									cursorTask = parentTask;
								}
							}
							else
							{
								GTA::CTask *newTask = (GTA::CTask*)cloned.task->GetTask();
								newTask->Deserialize(cloned.task);
								cursorTask->NextSubTask = newTask;
								cursorTask = newTask;
							}
						}
						log_debug << "Assigned: " << parentTask->GetTree() << std::endl;
						player->AssignTask(parentTask);

						for each (auto cloned in ClonedTasks)
							rage::sysMemAllocator::Get()->free((void*)cloned.task, rage::HEAP_TASK_CLONE);
					}
				}
				break;
			}
			case ID_PLAYER_LEFT:
			{
				RakNet::RakNetGUID guid;
				bsIn.Read(guid);
				CNetworkPlayer::DeleteByGUID(guid);
				break;
			}
			default:
			{
				log << "[RakNet] Unknown message id: " << (int)packet->data[0] << ", message: " << packet->data << std::endl;
				break;
			}
		}
	}

	for (packet = tcpclient->Receive(); packet; tcpclient->DeallocatePacket(packet), packet = tcpclient->Receive()) {
		unsigned char packetid;
		RakNet::BitStream bsIn(packet->data, packet->length, false);
		bsIn.Read(packetid);

		switch (packetid)
		{
		case 1:
		{
			log << "Client-side scripts: " << packet->length << std::endl;
			size_t count;

			bsIn.Read(count);

			for (int i = 0; i < count; i++)
			{
				RakString name;
				unsigned int size;
				bsIn.Read(name);
				bsIn.Read(size);

				unsigned char* code = (unsigned char*)malloc(size);
				bool result = bsIn.ReadAlignedBytes(code, size);

				CScriptEngine::Get()->LoadScript(size, reinterpret_cast<char*>(code), name.C_String());

				free(code);
			}

			cEstablished++;
			if (cEstablished > 1)
			{
				BitStream bsOut;
				RakString playerName(CConfig::Get()->sNickName.c_str());
				bsOut.Write((unsigned char)ID_CONNECT_TO_SERVER);
				bsOut.Write(playerName);

				client->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
			}

			break;
		}
		case 2:
			break;
		case 30:
			break;
		default:
			log << (int)packetid << " " << packet->length << std::endl << (char*)packet->data << std::endl;
		}
	}
}
