#pragma once
class CNetworkMasterServer
{
	static CNetworkMasterServer *singleInstance;

	CNetworkMasterServer();
public:
	~CNetworkMasterServer();

	bool bConnected = false;

	static CNetworkMasterServer * Get();

	bool Connect();
	void Disconnect();
	bool IsConnected() { return bConnected; }
	void Update();
	void Init();

	RakNet::RakPeerInterface *client;
	RakNet::ConnectionAttemptResult connection;
};

