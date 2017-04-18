#pragma once
class CNetworkManager
{
public:
	RakNet::Packet *packet;
	RakNet::RakPeerInterface *server;

	void Tick();
	CNetworkManager();
	~CNetworkManager();
};

