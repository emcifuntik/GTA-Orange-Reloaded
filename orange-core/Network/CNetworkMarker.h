#pragma once

class CNetworkMarker
{
	static std::vector<CNetworkMarker *> MarkerPool;

	CVector3 vecPos;
	float height;
	int color;
	int type;
	int Handle;
	float radius;

public:
	RakNet::RakNetGUID	m_GUID;
	CNetworkMarker(RakNetGUID guid, float x, float y, float z, float height, float radius);

	void SetColor(int color);
	void SetHeight(float height);

	static std::vector<CNetworkMarker*> All();
	static CNetworkMarker * GetByGUID(RakNet::RakNetGUID GUID);
	static void DeleteByGUID(RakNet::RakNetGUID GUID);

	~CNetworkMarker();
};
