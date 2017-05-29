#pragma once
class CNetworkManager
{
	static CNetworkManager * singleInstance;

	RakNet::SocketDescriptor socketDescriptors[2];

	CNetworkManager();
public:
	RakNet::Packet *packet;
	RakNet::RakPeerInterface *server;


	static CNetworkManager * Get();
	void Tick();
	bool Start();
	~CNetworkManager();
};

