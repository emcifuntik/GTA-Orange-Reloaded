#pragma once

class CNetworkConnection
{
	static CNetworkConnection * singleInstance;
	CNetworkConnection();

	RPC4 rpc;
	RakNet::Packet* packet;
	RakNet::SocketDescriptor socketDescriptors[2];
	RakNet::ConnectionAttemptResult connection;
	bool bConnected = false;
public:
	RakNet::RakPeerInterface *server;
	RakNet::PacketizedTCP *tcpserver;

	std::vector<Hash> UsedModels;

	static CNetworkConnection * Get();

	void Send(const RakNet::BitStream * bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, const AddressOrGUID systemIdentifier, bool broadcast, int radius);

	bool Start(unsigned short maxPlayers, unsigned short port);
	void Tick();

	~CNetworkConnection();
};