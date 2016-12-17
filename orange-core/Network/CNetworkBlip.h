#pragma once

class CNetworkBlip
{
	static std::vector<CNetworkBlip *> BlipPool;

	CVector3 vecPos;
	float scale;
	int color;
	int sprite;
	Blip Handle;
public:
	RakNet::RakNetGUID	m_GUID;
	CNetworkBlip(RakNetGUID guid, float x, float y, float z, float scale, int color, int sprite);

	void SetScale(float scale);
	void SetColor(int color);
	void SetAsShortRange(bool _short);
	void SetSprite(int sprite);
	void SetRoute(bool route);
	void AttachToPlayer(RakNet::RakNetGUID GUID);

	static std::vector<CNetworkBlip*> All();
	static CNetworkBlip * GetByGUID(RakNet::RakNetGUID GUID);

	~CNetworkBlip();
};

