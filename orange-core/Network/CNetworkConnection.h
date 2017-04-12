#pragma once
class CNetworkConnection
{
	static CNetworkConnection *singleInstance;

	CNetworkConnection();
public:
	~CNetworkConnection();

	bool bConnected = false;
	bool bEstablished = false;
	char cEstablished = 0;
	bool bClear = false;

	static CNetworkConnection * Get();

	bool Connect(std::string host, unsigned short port);
	void Disconnect();
	bool IsConnected() { return bConnected; }
	bool IsConnectionEstablished() { return bEstablished; }
	void Tick();

	RakNet::RakPeerInterface *client;
	RakNet::PacketizedTCP *tcpclient;
	RakNet::Packet* packet;
	RakNet::SystemAddress clientID;
	RakNet::ConnectionAttemptResult connection;
};

