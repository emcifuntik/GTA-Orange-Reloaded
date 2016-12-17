#pragma once
class CNetworkConnection
{
	static CNetworkConnection *singleInstance;

	CNetworkConnection();
	bool bConnected = false;
	bool bEstablished = false;
public:
	~CNetworkConnection();
	static CNetworkConnection * Get();

	bool Connect(std::string host, unsigned short port);
	void Disconnect();
	bool IsConnected() { return bConnected; }
	bool IsConnectionEstablished() { return bEstablished; }
	void Tick();

	RakNet::RakPeerInterface *client;
	RakNet::Packet* packet;
	RakNet::SystemAddress clientID;
	RakNet::ConnectionAttemptResult connection;
};

