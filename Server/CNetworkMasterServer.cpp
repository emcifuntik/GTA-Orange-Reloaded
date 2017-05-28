#include "stdafx.h"

CNetworkMasterServer *CNetworkMasterServer::singleInstance = nullptr;

struct MasterServerData
{
public:
	unsigned short Port;
	int Players;
	unsigned short MaxPlayers;
};

CNetworkMasterServer::CNetworkMasterServer()
{
	client = RakNet::RakPeerInterface::GetInstance();
}

CNetworkMasterServer::~CNetworkMasterServer()
{

}

CNetworkMasterServer * CNetworkMasterServer::Get()
{
	if (!singleInstance)
		singleInstance = new CNetworkMasterServer();
	return singleInstance;
}

bool CNetworkMasterServer::Connect()
{
	RakNet::SocketDescriptor socketDescriptor(0, 0);

	socketDescriptor.socketFamily = AF_INET;

	client->Startup(8, &socketDescriptor, 1);
	client->SetOccasionalPing(true);
	connection = client->Connect("51.254.128.180", 7755, 0, 0);
	RakAssert(connection == RakNet::CONNECTION_ATTEMPT_STARTED);

	if (connection == RakNet::CONNECTION_ATTEMPT_STARTED)
	{
		bConnected = true;
		log << "Connected to the Master-Server" << std::endl;
		return true;
	}
	log << "Not connected to the Master-Server" << std::endl;
	return false;
}

void CNetworkMasterServer::Disconnect()
{
	bConnected = false;
	client->Shutdown(300);
}

void CNetworkMasterServer::Init()
{
	BitStream con;
	RakNet::RakString Hostname(CConfig::Get()->Hostname.c_str());
	unsigned short Port = CConfig::Get()->Port;
	int Players = CNetworkPlayer::Count();
	unsigned short MaxPlayers = CConfig::Get()->MaxPlayers;
	con.Write((unsigned char)150);
	con.Write(Port);
	con.Write(Players);
	con.Write(MaxPlayers);
	con.Write(Hostname);

	client->Send(&con, HIGH_PRIORITY, RELIABLE, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}

void CNetworkMasterServer::Update()
{
	BitStream con;
	int Players = CNetworkPlayer::Count();
	con.Write((unsigned char)151);
	con.Write(Players);

	client->Send(&con, HIGH_PRIORITY, RELIABLE, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}