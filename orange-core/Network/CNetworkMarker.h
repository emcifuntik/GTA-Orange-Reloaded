#pragma once

class CNetworkMarker
{
	static std::vector<CNetworkMarker *> MarkerPool;

	CVector3 vecPos;
	float height;
	color_t color;
	int type;
	int Handle;
	float radius;
	float scale = 1.0f;

public:
	RakNet::RakNetGUID	m_GUID;
	CNetworkMarker(RakNetGUID guid, float x, float y, float z, float height, float radius, color_t color = { 255, 0, 0, 255 });

	void SetColor(color_t color);
	void SetHeight(float height);
	void SetScale(float scale);

	static std::vector<CNetworkMarker*> All();
	static CNetworkMarker * GetByGUID(RakNet::RakNetGUID GUID);
	static void DeleteByGUID(RakNet::RakNetGUID GUID);

	~CNetworkMarker();
};
