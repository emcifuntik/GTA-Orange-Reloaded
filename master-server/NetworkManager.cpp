#include "stdafx.h"

CNetworkManager * CNetworkManager::singleInstance = nullptr;

void CNetworkManager::Tick()
{
	for (packet = server->Receive(); packet; server->DeallocatePacket(packet), packet = server->Receive())
	{
		unsigned char packetIdentifier = packet->data[0];
		RakNet::BitStream bsIn(packet->data, packet->length, false);
		bsIn.IgnoreBytes(sizeof(unsigned char));

		switch (packetIdentifier)
		{
			case ID_DISCONNECTION_NOTIFICATION:
			{
				std::cout << "ID_DISCONNECTION_NOTIFICATION GUID: " << packet->guid.ToString() << std::endl;
				break;
			}
			case ID_NEW_INCOMING_CONNECTION:
			{
				std::cout << "ID_NEW_INCOMING_CONNECTION GUID: " << packet->guid.ToString() << std::endl;
				OrangeServer* Server = new OrangeServer(packet->guid);
				break;
			}
			case 150:
			{
				OrangeServer* Server = OrangeServer::GetByGUID(packet->guid);
				if (Server)
				{
					unsigned short Port;
					int Players;
					unsigned short MaxPlayers;
					RakNet::RakString Hostname;
					bsIn.Read(Port);
					bsIn.Read(Players);
					bsIn.Read(MaxPlayers);
					bsIn.Read(Hostname);
					Server->Hostname = Hostname;
					Server->Port = Port;
					Server->Players = Players;
					Server->MaxPlayers = MaxPlayers;
					Server->IP = packet->systemAddress.ToString(false);
					Server->hasPassword = false;
					Server->Gamemode = "Dev";
					Server->isVerified = false;
					Server->init = true;
				}
				
				std::cout << "ID_SERVER_INIT GUID: " << packet->guid.ToString() << std::endl;
				break;
			}
			case 151:
			{
				OrangeServer* Server = OrangeServer::GetByGUID(packet->guid);
				if (Server)
				{
					int Players;
					bsIn.Read(Players);
					Server->Players = Players;
				}

				std::cout << "ID_SERVER_UPDATE GUID: " << packet->guid.ToString() << std::endl;
				break;
			}
			case ID_CONNECTION_LOST:
			{
				std::cout << "ID_CONNECTION_LOST GUID: " << packet->guid.ToString() << std::endl;
				OrangeServer* Server = OrangeServer::GetByGUID(packet->guid);
				Server->~OrangeServer();
				break;
			}
		}
	}
}

CNetworkManager::CNetworkManager()
{
	server = RakNet::RakPeerInterface::GetInstance();
}


CNetworkManager::~CNetworkManager()
{
}

CNetworkManager * CNetworkManager::Get()
{
	if (!singleInstance)
		singleInstance = new CNetworkManager();
	return singleInstance;
}

bool CNetworkManager::Start()
{
	socketDescriptors[0].port = 7755;
	socketDescriptors[0].socketFamily = AF_INET;

	server->SetMaximumIncomingConnections(10000);

	bool result = server->Startup(10000, socketDescriptors, 1) == RakNet::RAKNET_STARTED;
	if (!result)
	{
		std::cout << "Master Server not started" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "Master Server started" << std::endl;

	server->SetTimeoutTime(15000, RakNet::UNASSIGNED_SYSTEM_ADDRESS);

	return true;
}