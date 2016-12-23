#pragma once
class CNetworkBlip
{
	static std::vector<CNetworkBlip *> AllBlips;
	CVector3 vecPos;
	float scale;
	int color;
	int sprite;
	int playerid;
public:
	RakNet::RakNetGUID rnGUID;
	static std::vector<CNetworkBlip *> All();
	CNetworkBlip(float x, float y, float z, float scale, int color, int sprite, int playerid);

	void SetScale(float _scale);

	int GetPlayerID() { return playerid; };

	void Delete();

	~CNetworkBlip();
	static void SendGlobal(RakNet::Packet * packet);
	static CNetworkBlip * GetByGUID(RakNetGUID guid);
};

