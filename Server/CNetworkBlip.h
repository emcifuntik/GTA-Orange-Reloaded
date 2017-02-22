#pragma once
class CNetworkBlip
{
	static std::vector<CNetworkBlip *> AllBlips;
	CVector3 vecPos;
	float scale;
	std::string name;
	int color;
	int sprite;
	int playerid;
	bool bnear;
	bool route;
public:
	RakNet::RakNetGUID rnGUID;
	static std::vector<CNetworkBlip *> All();
	CNetworkBlip(std::string name, float x, float y, float z, float scale, int color, int sprite, int playerid);

	void SetScale(float _scale);
	void SetColor(int _color);
	void SetSprite(int _sprite);
	void SetRoute(bool _route);
	void SetName(std::string _name);
	void SetAsShortRange(bool _toggle);

	int GetPlayerID() { return playerid; };

	~CNetworkBlip();
	static void SendGlobal(RakNet::Packet * packet);
	static CNetworkBlip * GetByGUID(RakNetGUID guid);
	static void Delete(RakNetGUID guid);
};

