#include "stdafx.h"

CNetworkConnection * CNetworkConnection::singleInstance = nullptr;


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

CNetworkConnection::CNetworkConnection()
{
	server = RakNet::RakPeerInterface::GetInstance();
}

CNetworkConnection * CNetworkConnection::Get()
{
	if (!singleInstance)
		singleInstance = new CNetworkConnection();
	return singleInstance;
}

CNetworkConnection::~CNetworkConnection()
{
	server->Shutdown(300);
	RakNet::RakPeerInterface::DestroyInstance(server);
}

void CNetworkConnection::Send(const RakNet::BitStream * bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, const AddressOrGUID systemIdentifier, bool broadcast, int radius = 0)
{
	if (broadcast && radius != 0)
	{
		if (systemIdentifier.rakNetGuid == UNASSIGNED_RAKNET_GUID) return;

		auto player = CNetworkPlayer::GetByGUID(systemIdentifier.rakNetGuid);

		if (!player) return;

		for (auto pl : CNetworkPlayer::All())
		{
			if ((player->GetPosition() - pl->GetPosition()).Length() < radius) {
				server->Send(bitStream, priority, reliability, orderingChannel, pl->GetGUID(), false);
			}
		}

	}
	else server->Send(bitStream, priority, reliability, orderingChannel, systemIdentifier, broadcast);
}

bool CNetworkConnection::Start(unsigned short maxPlayers, unsigned short port)
{
	if (maxPlayers && port)
	{
		socketDescriptors[0].port = port;
		socketDescriptors[0].socketFamily = AF_INET; // Test out IPV4
		socketDescriptors[1].port = port;
		socketDescriptors[1].socketFamily = AF_INET6; // Test out IPV6
		bool result = server->Startup(maxPlayers, socketDescriptors, 2) == RakNet::RAKNET_STARTED;
		server->SetMaximumIncomingConnections(maxPlayers);
		if (!result)
		{
			result = server->Startup(maxPlayers, socketDescriptors, 1) == RakNet::RAKNET_STARTED;
			if (!result)
			{
				log << "Server not started" << std::endl;
				exit(EXIT_FAILURE);
			}
			else
				log << "Server started" << std::endl;
		}
		server->SetTimeoutTime(15000, RakNet::UNASSIGNED_SYSTEM_ADDRESS);
		return true;
	}
	return false;
}

void CNetworkConnection::Tick()
{
	for (packet = server->Receive(); packet; server->DeallocatePacket(packet), packet = server->Receive())
	{
		unsigned char packetIdentifier = packet->data[0];
		RakNet::BitStream bsIn(packet->data, packet->length, false);
		bsIn.IgnoreBytes(sizeof(unsigned char));
		RakNet::BitStream bsOut;

		switch (packetIdentifier)
		{
			case ID_DISCONNECTION_NOTIFICATION:
			{
				log << "Player disconnected " << packet->systemAddress.ToString(true) << std::endl;

				CNetworkPlayer *player = CNetworkPlayer::GetByGUID(packet->guid);
				UINT playerID = player->GetID();

				Plugin::PlayerDisconnect(playerID, 1);

				CNetworkPlayer::Remove(playerID);

				bsOut.Write((unsigned char)ID_PLAYER_LEFT);
				bsOut.Write(packet->guid);

				server->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->guid, true);
				break;
			}
			case ID_NEW_INCOMING_CONNECTION:
			{
				log << "Incoming connection from " << packet->systemAddress.ToString(true) << std::endl;
				bsOut.Write(UsedModels.size());
				for (Hash m : UsedModels) bsOut.Write(m);
				CRPCPlugin::Get()->Signal("PreloadModels", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, packet->systemAddress, false, false);

				CNetworkBlip::SendGlobal(packet);
				CNetworkMarker::SendGlobal(packet);
				CNetworkVehicle::SendGlobal(packet);
				CNetwork3DText::SendGlobal(packet);
				CNetworkObject::SendGlobal(packet);
				CClientScripting::SendGlobal(packet);

				break;
			}
			case ID_CONNECT_TO_SERVER:
			{
				RakNet::RakString playerName;
				bsIn.Read(playerName);
				CNetworkPlayer *player = new CNetworkPlayer(packet->guid);
				player->SetName(playerName.C_String());

				Plugin::PlayerConnect(player->GetID());
				Plugin::Trigger("PlayerConnect", (unsigned long)player->GetID());
				
				bsOut.Write((unsigned char)ID_CONNECT_TO_SERVER);
				server->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
				break;
			}
			case ID_CHAT_MESSAGE:
			{
				RakNet::RakString playerText;
				bsIn.Read(playerText);

				if (Plugin::PlayerText(CNetworkPlayer::GetByGUID(packet->guid)->GetID(), playerText.C_String()))
				{
					std::stringstream ss;
					ss << CNetworkPlayer::GetByGUID(packet->guid)->GetName() << ": " << playerText.C_String();
					RakNet::RakString toSend(ss.str().c_str());
					bsOut.Write(toSend);
					color_t messageColor = { 200, 200, 255, 255 };
					bsOut.Write(messageColor);
					CRPCPlugin::Get()->Signal("SendClientMessage", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, false);
				}
				break;
			}
			case ID_COMMAND_MESSAGE:
			{
				RakNet::RakString playerText;
				bsIn.Read(playerText);

				/*std::vector<std::string> cmdArgs = split(playerText.C_String(), ' ');
				std::string cmd = cmdArgs[0].substr(1);
				cmdArgs.erase(cmdArgs.begin(), cmdArgs.begin() + 1);*/

				if (Plugin::PlayerCommand(CNetworkPlayer::GetByGUID(packet->guid)->GetID(), playerText.C_String()))
				{
					RakNet::RakString toSend("Unknown command");
					bsOut.Write(toSend);
					color_t messageColor = { 255, 200, 200, 255 };
					bsOut.Write(messageColor);
					CRPCPlugin::Get()->Signal("SendClientMessage", &bsOut, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, packet->guid, false, false);
				}
				break;
			}
			case ID_SEND_PLAYER_DATA:
			{
				CNetworkPlayer *player = CNetworkPlayer::GetByGUID(packet->guid);
				OnFootSyncData data;
				bsIn.Read(data);

				//if(data.bInVehicle) log << "(" << player->GetID() << ") seat: " << data.vehseat << std::endl;
				player->SetOnFootData(data);
				
				if (!Plugin::PlayerUpdate(CNetworkPlayer::GetByGUID(packet->guid)->GetID()))
					continue;

				bsOut.Write((unsigned char)ID_SEND_PLAYER_DATA);
				bsOut.Write(packet->guid);
				RakNet::RakString rsName(player->GetName().c_str());
				bsOut.Write(rsName);

				player->GetOnFootData(data);
#if _DEBUG
				data.vecPos.fX += 1.f;
				data.vecPos.fY += 1.f;

				if(data.bInVehicle)
					for each(auto *veh in CNetworkVehicle::All())
					{
						if (veh->GetGUID() != data.vehicle) {
							data.vehicle = veh->GetGUID();
							break;
						}
					}
#endif
				bsOut.Write(data);
#if _DEBUG

				server->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
#else
				server->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true);
#endif
				bsOut.Reset();
				break;
			}
			case ID_SEND_VEHICLE_DATA:
			{
				VehicleData data;
				bsIn.Read(data);

				if (data.GUID == UNASSIGNED_RAKNET_GUID) continue;

				if(data.hasDriver) data.driver = packet->guid;

				bsOut.Write((unsigned char)ID_SEND_VEHICLE_DATA);
#if _DEBUG
				data.vecPos.fX += 4;
				data.vecPos.fY += 4;

				for each(auto *veh in CNetworkVehicle::All())
				{
					if (veh->GetGUID() != data.GUID) {
						data.GUID = veh->GetGUID();
						break;
					}
				}
#endif				
				CNetworkVehicle *veh = CNetworkVehicle::GetByGUID(data.GUID);
					
				veh->SetVehicleData(data);
				veh->GetVehicleData(data);

				bsOut.Write(data);

#if _DEBUG
				server->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
#else
				server->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true);
#endif
				bsOut.Reset();
				break;
			}
			case ID_SEND_TASKS:
			{
				int tasks = 0;
				bsIn.Read(tasks);
				bsOut.Write((unsigned char)ID_SEND_TASKS);
				bsOut.Write(packet->guid);
				bsOut.Write(tasks);
				for (int i = 0; i < tasks; ++i)
				{
					unsigned short taskID;

					bsIn.Read(taskID);
					bsOut.Write(taskID);

					unsigned int size;

					bsIn.Read(size);
					bsOut.Write(size);

					int bytesSize = (size % 8) ? (size / 8 + 1) : (size / 8);

					unsigned char* taskInfo = new unsigned char[bytesSize];
					bsIn.ReadBits(taskInfo, size);
					bsOut.WriteBits(taskInfo, size);
					delete[] taskInfo;
				}
#if _DEBUG
				server->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
#else
				server->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, true);
#endif
				bsOut.Reset();
				break;
			}
			case ID_CONNECTED_PING:
			case ID_UNCONNECTED_PING:
			{
				log << "Ping from " << packet->systemAddress.ToString(true) << std::endl;
				break;
			}
			case ID_CONNECTION_LOST:
			{
				log << "Connection with " << packet->systemAddress.ToString(true) << " lost" << std::endl;
				CNetworkPlayer *player = CNetworkPlayer::GetByGUID(packet->guid);
				UINT playerID = player->GetID();

				Plugin::PlayerDisconnect(playerID, 2);
				Plugin::Trigger("PlayerDisconnect", (unsigned long)playerID, 2);

				CNetworkPlayer::Remove(playerID);

				bsOut.Write((unsigned char)ID_PLAYER_LEFT);
				bsOut.Write(packet->guid);

				server->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
				break;
			}
			default:
			{
				log << "Unknown packet identifier: " << packet->data << std::endl;
				break;
			}
		}
	}
}
