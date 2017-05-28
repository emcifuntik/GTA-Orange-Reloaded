#pragma once
class OrangeServer
{
	static std::vector<OrangeServer *> ServerList;

public:

	RakNet::RakNetGUID GUID;
	unsigned short Port;
	int Players;
	unsigned short MaxPlayers;
	RakNet::RakString Hostname;
	RakNet::RakString IP;
	RakNet::RakString Gamemode;
	bool hasPassword;
	bool isVerified;
	bool init = false;

	static std::vector<OrangeServer *> All();
	static int Count();
	static OrangeServer *GetByGUID(RakNet::RakNetGUID);

	void Update();
	void Init();

	OrangeServer(RakNet::RakNetGUID);
	~OrangeServer();
};

