#pragma once

class CNetworkBlip
{
	static std::vector<CNetworkBlip *> BlipPool;

	CVector3 vecPos;
	std::string name;
	float scale;
	int color;
	int sprite;

	RakNetGUID attachedTo;
	char attachType;
public:
	Blip Handle;
	RakNet::RakNetGUID	m_GUID;
	CNetworkBlip(RakNetGUID guid, std::string name, float x, float y, float z, float scale, int color, int sprite);

	void SetScale(float scale);
	void SetColor(int color);
	void SetAsShortRange(bool _short);
	void SetSprite(int sprite);
	void SetRoute(bool route);
	void SetName(std::string name);

	void AttachToPlayer(RakNet::RakNetGUID GUID);
	void AttachToVehicle(RakNet::RakNetGUID GUID);

	void Update();

	static std::vector<CNetworkBlip*> All();
	static CNetworkBlip * GetByGUID(RakNet::RakNetGUID GUID);

	~CNetworkBlip();
};

